#ifndef PATTERNMATCH_MSQTREE_H
#define PATTERNMATCH_MSQTREE_H

#include "qtreebase.h"

// box idxes stored only in leaves
class MSQtree : public QtreeBase
{
    // 它是一个四叉树的实现，用来在二维空间中高效地进行查询和存储。
public:
    MSQtree(const IndexBoxes &container, NodeBounds root_bounds, int depth, sptr<const HotSpotMap> hot_spot_map = std::make_shared<HotSpotMap>());
    // hot_spot_map 是一个指向 const HotSpotMap 对象的智能指针。= std::make_shared<HotSpotMap>()： 这部分代码用来初始化 hot_spot_map，将它设置为指向一个通过 std::make_shared<HotSpotMap>() 创建的新 HotSpotMap 对象。
    //  root_bounds：NodeBounds 类型，表示四叉树根节点的边界框，定义了树的根节点覆盖的空间范围。
    void queryInner(BoundingBox search_box, std::vector<PolygonId> &ret);
    // 传入一个 search_box（表示查询的边界框）和一个 ret 容器，用来存储查询结果。该方法的功能是查找树中完全包含在 search_box 中的对象（如多边形）。

    void queryIntersect(std::vector<uint> &nodes, BoundingBox search_box, std::vector<PolygonId> &ret);
    // 查找与 search_box 相交的节点，并将结果保存在 ret 中。nodes 记录了哪些节点在进行查询时被检查。

    int queryInnerCount(std::vector<uint> &nodes, BoundingBox search_box);
    // 返回完全在 search_box 内的节点数量

    std::vector<uint> getIntersectNodes(BoundingBox search_box) const;
    // 返回与 search_box 相交的所有节点索引的列表

protected:
    void buildSubtree(uint node_idx, NodeBounds bounds, int depth);
    // 这个方法用于递归地构建四叉树的子树。给定一个节点索引 node_idx，它会使用 bounds 来确定当前节点的边界，并根据 depth 控制递归深度

protected:
    const int depth_; // depth=8
    const NodeBounds root_bounds_;
    const IndexBoxes &ibox_container_; // 存一个Tcontainer元素的iboxes
    const uint poly_num_;              // 记录ibox_container的个数
    sptr<const HotSpotMap> is_hot_spot_;
    // template <class T>
    // using sptr = std::shared_ptr<T>;

    size_t off_{};                             //
    std::vector<NodeContent> node_contents_{}; // 拥有1 << (2 * depth)个元素，代表这么多个cell，负责存每个热点cell包含的layout的poly的序号
    // 通过ibox_container_可以查到对应poly的ibox，再通过index访问layout文件，从而得到这个poly的点
};

class MSQtreeManager
{
protected:
    using HotSpot = Marker;
    using HotSpots = std::vector<HotSpot>; // Markers
    enum TreeType
    {
        LayoutTree,
        FilterTree
    };

public:
    // MSQtreeManager(Layout &layout, const TContainer<HotSpots> &hotspots, Range inner_count);
    MSQtreeManager(Layout &layout, const HotSpots &hotspots, int layer_label);

    // MSQtreeManager(TContainer<IndexBoxes> &iboxes, TContainer<BoundingBox> &container_box);

    std::vector<PolygonId> query(BoundingBox search_box);

    std::vector<uint> getIntersectNodes(BoundingBox search_box) { return qtree_->getIntersectNodes(search_box); }

protected:
    BoundingBox qtree_box_;
    ptr<MSQtree> qtree_;
    int qt_layer_label;
    // TContainer<BoundingBox> qtree_box_;
    // TContainer<ptr<MSQtree>> qtree_; // 8个MSQtree的智能指针
    // template <class T> using ptr = std::unique_ptr<T>;
    // ptr<MSQtree>：ptr 是对 std::unique_ptr<T> 的别名，表示一个管理 MSQtree 对象生命周期的智能指针。
    // std::unique_ptr 是 C++11 中提供的智能指针，用于独占地管理对象的动态内存，确保对象在不再需要时被自动释放。
    // Range inner_count_{};
    TreeType tree_type_;
};

#endif // PATTERNMATCH_MSQTREE_H
