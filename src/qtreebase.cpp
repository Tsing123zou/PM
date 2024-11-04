#include "qtreebase.h"
// #include "Thread.h"

QtreeBase::NodeBounds::NodeBounds(BoundingBox box) : cx((vert_t)box.cx()), cy((vert_t)box.cy())
{
    int wx = 0;
    vert_t thw = box.width() / 2 + 1;
    while (thw)
    {
        thw >>= 1;
        wx++;
    }
    hw = 1 << wx;

    int hx = 0;
    vert_t thh = box.height() / 2 + 1;
    while (thh)
    {
        thh >>= 1;
        hx++;
    }
    hh = 1 << hx;
}

QtreeBase::NodeBounds QtreeBase::NodeBounds::childBounds(QtreeBase::ChildIndex ci) const
{
    vert_t hhw = hw >> 1, hhh = hh >> 1;
    vert_t tcx = cx, tcy = cy;
    if (ci == ll)
    {
        tcx -= hhw;
        tcy -= hhh;
    }
    else if (ci == lr)
    {
        tcx += hhw;
        tcy -= hhh;
    }
    else if (ci == ul)
    {
        tcx -= hhw;
        tcy += hhh;
    }
    else if (ci == ur)
    {
        tcx += hhw;
        tcy += hhh;
    }
    return {tcx, tcy, hhw, hhh};
}

bool QtreeBase::NodeBounds::intersect(const BoundingBox &box) const
{
    return box.lx < rx() && lx() < box.rx && box.ly < uy() && ly() < box.uy;
}

bool QtreeBase::NodeBounds::contain(const BoundingBox &box) const
{
    return lx() < box.lx && box.rx < rx() && ly() < box.ly && box.uy < uy();
}

QtreeBase::Coordinate2Index QtreeBase::C2I;

uint QtreeBase::Coordinate2Index::operator()(uint x, uint y) const
{
    uint res = 0;
    int i = 0;
    while (x || y)
    {
        uint tx = x % (1 << N);
        uint ty = y % (1 << N);
        res |= C2I[ty * (1 << N) + tx] << (2 * i * N);
        x >>= N;
        y >>= N;
        i++;
    }
    return res;
}

void QtreeBase::Coordinate2Index::build(uint x, uint y, uint cur_idx, int depth)
{
    if (depth == N)
    {
        C2I[y * (1 << N) + x] = cur_idx;
        return;
    }
    build(x * 2 + 0, y * 2 + 0, cur_idx * 4 + ll, depth + 1); // ll
    build(x * 2 + 1, y * 2 + 0, cur_idx * 4 + lr, depth + 1); // lr
    build(x * 2 + 0, y * 2 + 1, cur_idx * 4 + ul, depth + 1); // ul
    build(x * 2 + 1, y * 2 + 1, cur_idx * 4 + ur, depth + 1); // ur
}

QtreeBase::HotSpotMap::HotSpotMap(NodeBounds root_bounds, int depth, const HotSpots &hot_spots)
{
    // 根据hot_spots来更新map
    size_t off = 0;
    for (int i = 0; i < depth; i++)
        off += 1 << (2 * i);
    map_.resize(1 << (2 * depth), false);

    const auto &markHotSpot = [&](const HotSpot &hot_spot)
    {
        std::deque<Node> nodes{{0, 0, root_bounds}};
        while (!nodes.empty())
        {
            auto [idx, dep, bounds] = nodes.front();
            nodes.pop_front();

            if (dep != depth)
            {
                uint first_child = idx * 4 + 1;
                for (int i = ll; i <= ur; i++)
                {
                    auto ci = static_cast<ChildIndex>(i);
                    NodeBounds child_bounds = bounds.childBounds(ci);
                    if (child_bounds.intersect(hot_spot))
                    {
                        nodes.emplace_back(first_child + i, dep + 1, child_bounds);
                    }
                }
            }
            else
            {
                map_[idx - off] = true;
            }
        }
    };
    const auto &markHotSpots = [&](const HotSpots &hot_spots)
    {
        for (auto &hot_spot : hot_spots)
        {
            markHotSpot(hot_spot);
        }
    };

    // Thread::runTasks(markHotSpots, hot_spots);
    markHotSpots(hot_spots);
}

bool QtreeBase::HotSpotMap::operator()(uint idx) const
{
    if (idx >= map_.size())
        return true;
    return map_[idx];
}