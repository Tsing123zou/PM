#ifndef PATTERNMATCH_QTREEBASE_H
#define PATTERNMATCH_QTREEBASE_H

#include <vector>
#include <array>
#include <unordered_set>

#include "geometry.h"
#include "layout.h"

class QtreeBase
{
    // 四叉树（Quadtree）基础类 QtreeBase
    // 四叉树是一种用于分割二维空间的树结构，常用于处理空间搜索、碰撞检测或区域查询等操作。
    friend class Logger; //// 允许 Logger 访问 QtreeBase 的私有和保护成员

public:
    using HotSpot = Marker;
    using HotSpots = std::vector<HotSpot>;

    enum ChildIndex
    {
        // 定义子节点索引，表示四叉树节点的四个象限：左下、右下、左上、右上
        ll,
        lr,
        ul,
        ur
    };

    struct NodeBounds
    {
        // 用于表示四叉树节点的边界。
        vert_t cx{}, cy{}, hw{}, hh{};

        NodeBounds() = default;

        NodeBounds(const NodeBounds &other) = default;

        NodeBounds(vert_t cx, vert_t cy, vert_t hw, vert_t hh) : cx(cx), cy(cy), hw(hw), hh(hh) {}

        explicit NodeBounds(BoundingBox box);

        NodeBounds childBounds(ChildIndex ci) const;

        bool intersect(const BoundingBox &box) const; // 与box有交叉

        bool contain(const BoundingBox &box) const; // 包含box

        // 分别返回节点的左右边界和上下边界。
        vert_t lx() const { return cx - hw; }
        vert_t rx() const { return cx + hw; }
        vert_t ly() const { return cy - hh; }
        vert_t uy() const { return cy + hh; }
    };

    class HotSpotMap
    {
    public:
        HotSpotMap() = default; ////对于默认值构造时，map_为空

        explicit HotSpotMap(NodeBounds root_bounds, int depth, const HotSpots &hot_spots_container);
        // 构造函数通过根节点的边界和深度，初始化热点地图 map_。
        bool operator()(uint idx) const; // 用于根据索引检查某个位置是否有热点

    private:
        std::vector<bool> map_; // 每个元素表示一个位置是否有热点。
    };

protected:
    struct NodeContent
    {
        std::vector<uint> ibox_idxes{};
    };

    // temp node
    struct Node
    {
        uint idx{};        // 节点的索引
        int depth{};       // 节点在树中的深度
        NodeBounds bounds; // 节点的边界信息
                           // 添加构造函数
        Node(uint idx, int depth, const NodeBounds &bounds)
            : idx(idx), depth(depth), bounds(bounds) {}
    };

public:
    // mapping coordinate at grid to tree node elements vector
    class Coordinate2Index
    {
    public:
        Coordinate2Index() noexcept { build(0u, 0u, 0u, 0u); };
        // noexcept 是 C++ 中的一种函数声明规范，用来表明函数不会抛出异常。
        // 如果在执行标记为 noexcept 的函数时发生了异常，程序将直接终止，而不是通过正常的异常传播机制处理。
        // 一般来说，在性能敏感的代码中，noexcept 可以帮助编译器生成更高效的代码，因为它知道不需要为异常处理做额外的准备。

        uint operator()(uint x, uint y) const;

    private:
        void build(uint x, uint y, uint cur_idx, int depth);

        static const int N = 5; // N*N matrix
        std::array<uint, 1 << (2 * N)> C2I{};
        // 这种结构在一些空间划分算法（如四叉树或八叉树等）中很常见，用来存储大量预计算的索引值或坐标映射。
        // std::array 是 C++ 标准库中的一个模板类，用于表示一个固定大小的数组。
        // 相比普通的 C 风格数组，std::array 提供了更多的接口和类型安全特性。
    };

    static Coordinate2Index C2I; // Qtreebase构造时，仅有该部分初始化
    // static 成员变量属于类本身，而不是类的实例。会在 QtreeBase 类的所有实例之间共享
};

#endif // PATTERNMATCH_QTREEBASE_H
