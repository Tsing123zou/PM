#include "msqtree.h"
// #include "Logger.h"
// #include "Thread.h"

// MSQtreeManager::MSQtreeManager(Layout &layout, const TContainer<HotSpots> &hot_spots, Range inner_count) : tree_type_(LayoutTree), inner_count_(inner_count), qtree_box_(layout.container_box_)
// {
//     int depth = 8;
//     QtreeBase::NodeBounds root_bounds(layout.layout_box_);
//     // TICK(markHotSpots);
//     //  hot_spots是热点markers区域，范围与pattern的marker相同
//     sptr<QtreeBase::HotSpotMap> hot_spot_map(new QtreeBase::HotSpotMap(root_bounds, depth, hot_spots));
//     //    I4TOCK(markHotSpots);
//     const auto &buildLayoutQtree = [&](const auto &iboxes, auto &qtree)
//     {
//         qtree = std::make_unique<MSQtree>(iboxes, root_bounds, depth, hot_spot_map);
//     };
//     // 多线程时，8个qtree的root_bounds, depth, hot_spot_map相同，iboxes不同
//     Thread::runTasks(buildLayoutQtree, layout.iboxes_, qtree_);
// }

MSQtreeManager::MSQtreeManager(Layout &layout, const HotSpots &hot_spots, int layer_label) : tree_type_(LayoutTree), qtree_box_(layout.layout_box_[layer_label]), qt_layer_label(layer_label)
{
    int depth; // 需要根据layout_layout的poly数量进行修改
    if (layout.layer_poly_num[qt_layer_label] < 10000)
        depth = 6;
    else if (layout.layer_poly_num[qt_layer_label] < 100000)
        depth = 7;
    else
        depth = 8;
    QtreeBase::NodeBounds root_bounds(layout.layout_box_[qt_layer_label]);

    // TICK(markHotSpots);
    //  hot_spots是热点markers区域，范围与pattern的marker相同
    sptr<QtreeBase::HotSpotMap> hot_spot_map(new QtreeBase::HotSpotMap(root_bounds, depth, hot_spots));
    //    I4TOCK(markHotSpots);

    // const auto &buildLayoutQtree = [&](const auto &iboxes, auto &qtree)
    // {
    //     qtree = std::make_unique<MSQtree>(iboxes, root_bounds, depth, hot_spot_map);
    // };
    // // 多线程时，8个qtree的root_bounds, depth, hot_spot_map相同，iboxes不同
    // Thread::runTasks(buildLayoutQtree, layout.iboxes_, qtree_);
    qtree_ = std::make_unique<MSQtree>(layout.layout_iboxes, root_bounds, depth, hot_spot_map);
}

// MSQtreeManager::MSQtreeManager(TContainer<IndexBoxes> &iboxes, TContainer<BoundingBox> &container_box) : tree_type_(FilterTree)
// {
//     const auto &buildFilterQtree = [&](const auto &iboxes, auto &qtree, auto &qtree_box, auto &container_box)
//     {
//         qtree_box = container_box;
//         QtreeBase::NodeBounds root_bounds(qtree_box);
//         qtree = std::make_unique<MSQtree>(iboxes, root_bounds, 7); // 智能指针
//     };
//     Thread::runTasks(buildFilterQtree, iboxes, qtree_, qtree_box_, container_box);
// }

std::vector<PolygonId> MSQtreeManager::query(BoundingBox search_box) // 输入是layout中的marker
{
    // const auto &forEachIntersectQtree = [&](const std::function<void(MSQtree *)> &func)
    // {
    //     for (int i = 0; i < qtree_.size(); i++)
    //     {
    //         // qtree_是MSQtree的指针
    //         // qtree_box_是qtree指针中包含的iboxes的box
    //         // 一个search_box可能与多个qtree_box_相交
    //         if (search_box.intersect(qtree_box_[i]))
    //         {
    //             func(qtree_[i].get()); // qtree_[i].get() 提供智能指针的原始指针给 func 使用。
    //         }
    //     }
    // };
    // 没有inner_count_；
    //  const auto &innerCountMatch = [&](std::vector<uint> nodes, BoundingBox search_box)
    //  {
    //      int count = 0;
    //      forEachIntersectQtree([&](MSQtree *qtree)
    //                            { count += qtree->queryInnerCount(nodes, search_box); });
    //      return inner_count_.contain(count);
    //  };

    std::vector<PolygonId> ret;
    auto nodes = getIntersectNodes(search_box);
    qtree_->queryIntersect(nodes, search_box, ret);
    // if (tree_type_ == LayoutTree)
    // {
    //     auto nodes = getIntersectNodes(search_box);
    //     if (inner_count_.min_ <= 1 || innerCountMatch(nodes, search_box))
    //     {
    //         forEachIntersectQtree([&](MSQtree *qtree)
    //                               { qtree->queryIntersect(nodes, search_box, ret); });
    //     }
    // }
    // else if (tree_type_ == FilterTree)
    // {
    //     // 对于FilterTree,query是得到在search_box区域内的ibox的pid
    //     forEachIntersectQtree([&](MSQtree *qtree)
    //                           { qtree->queryInner(search_box, ret); });
    // }
    return ret;
}

MSQtree::MSQtree(const IndexBoxes &container, NodeBounds root_bounds, int depth, sptr<const HotSpotMap> hot_spot_map) : ibox_container_(container),
                                                                                                                        poly_num_(container.size()),
                                                                                                                        root_bounds_(root_bounds),
                                                                                                                        depth_(depth), // 对于layout为8，对于filter是7
                                                                                                                        node_contents_(1 << (2 * depth)),
                                                                                                                        is_hot_spot_(std::move(hot_spot_map)) // filter无该指针
{
    for (int i = 0; i < depth; i++)
        off_ += 1 << (2 * i);

    // TICK(buildMainTree);
    buildSubtree(0, root_bounds, depth_);
    //    I4TOCK(buildMainTree);
}

void MSQtree::buildSubtree(uint node_idx, NodeBounds bounds, int depth)
{
    if (depth <= 0)
        return;

    // partition bbox array
    // 对于layout depth=8
    int numSeg = 1 << depth; // num of segments that edge divided into
    vert_t cell_width = bounds.hw / numSeg * 2;
    vert_t cell_height = bounds.hh / numSeg * 2;
    vert_t root_llx = bounds.cx - bounds.hw;
    vert_t root_lly = bounds.cy - bounds.hh;
    for (uint i = 0; i < poly_num_; i++)
    {
        auto &bbox = ibox_container_[i];
        vert_t lx = bbox.lx - root_llx;
        vert_t rx = bbox.rx - root_llx - 1;
        vert_t ly = bbox.ly - root_lly;
        vert_t uy = bbox.uy - root_lly - 1;
        uint lxi = lx / cell_width;
        uint rxi = rx / cell_width;
        uint lyi = ly / cell_height;
        uint uyi = uy / cell_height;

        // search the level and position of bbox
        for (uint row = lyi; row <= uyi; row++)
        {
            for (uint col = lxi; col <= rxi; col++)
            {
                uint idx = C2I(col, row);
                if ((*is_hot_spot_)(idx)) // 对于FilterTree，map_为空，这里一直返回true
                {
                    node_contents_[idx].ibox_idxes.push_back(i);
                }
            }
        }
    }
}

void MSQtree::queryInner(BoundingBox search_box, std::vector<PolygonId> &ret)
{
    std::set<PolygonId> set;
    const auto &rangeQueryNode = [&](std::deque<Node> nodes)
    {
        while (!nodes.empty())
        {
            auto [idx, dep, bounds] = nodes.front();
            nodes.pop_front();

            if (dep != depth_)
            {
                uint first_child = idx * 4 + 1;
                for (int i = ll; i <= ur; i++)
                {
                    auto ci = static_cast<ChildIndex>(i);
                    NodeBounds child_bounds = bounds.childBounds(ci);
                    if (child_bounds.intersect(search_box))
                    {
                        nodes.emplace_back(first_child + i, dep + 1, child_bounds);
                    }
                }
            }
            else
            {
                // 取最后一层中与search_box相交的idx（cell）
                for (auto i : node_contents_[idx - off_].ibox_idxes)
                {
                    auto &ibox = ibox_container_[i];
                    if (ibox.in(search_box))
                    {
                        set.insert(ibox.pid);
                    }
                }
            }
        }
    };

    rangeQueryNode({Node(0, 0, root_bounds_)});
    ret.insert(ret.end(), set.begin(), set.end());
}

void MSQtree::queryIntersect(std::vector<uint> &nodes, BoundingBox search_box, std::vector<PolygonId> &ret)
{
    std::set<PolygonId> set;
    for (auto idx : nodes)
    {
        for (auto i : node_contents_[idx].ibox_idxes)
        {
            auto &ibox = ibox_container_[i];
            if (ibox.intersect(search_box))
            {
                set.insert(ibox.pid);
            }
        }
    }
    ret.insert(ret.end(), set.begin(), set.end());
}

int MSQtree::queryInnerCount(std::vector<uint> &nodes, BoundingBox search_box)
{
    std::set<PolygonId> set;
    for (auto idx : nodes)
    {
        for (auto i : node_contents_[idx].ibox_idxes)
        {
            auto &ibox = ibox_container_[i];
            if (ibox.in(search_box))
            {
                set.insert(ibox.pid);
            }
        }
    }
    return (int)set.size();
}

std::vector<uint> MSQtree::getIntersectNodes(BoundingBox search_box) const
{
    // 查找与marker相交的cells
    //  返回
    std::vector<uint> ret;

    int numSeg = 1 << depth_; // num of segments that edge divided into
    vert_t cell_width = root_bounds_.hw / numSeg * 2;
    vert_t cell_height = root_bounds_.hh / numSeg * 2;
    vert_t root_llx = root_bounds_.cx - root_bounds_.hw;
    vert_t root_lly = root_bounds_.cy - root_bounds_.hh;

    auto &bbox = search_box;
    vert_t lx = bbox.lx - root_llx;
    vert_t rx = bbox.rx - root_llx - 1;
    vert_t ly = bbox.ly - root_lly;
    vert_t uy = bbox.uy - root_lly - 1;
    uint lxi = lx / cell_width;
    uint rxi = rx / cell_width;
    uint lyi = ly / cell_height;
    uint uyi = uy / cell_height;

    // search the level and position of bbox
    for (uint row = lyi; row <= uyi; row++)
    {
        for (uint col = lxi; col <= rxi; col++)
        {
            ret.push_back(C2I(col, row)); // C2I(col, row)得到的是索引值idx
        }
    }

    return ret;
}