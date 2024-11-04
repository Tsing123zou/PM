#ifndef PATTERNMATCH_GEOMETRY_H
#define PATTERNMATCH_GEOMETRY_H

#include <vector>

#include "clipper.h"

#include "types.h"

struct Vertex
{ // 定义整数点
    vert_t x{}, y{};

    Vertex operator-(const Vertex &o) const { return {x - o.x, y - o.y}; }

    void operator-=(const Vertex &o)
    {
        x -= o.x;
        y -= o.y;
    }

    bool operator==(const Vertex &o) const
    {
        return x == o.x && y == o.y;
    }
};

using Offset = Vertex;

struct VertexF
{ // 定义浮点数的点
    double x{}, y{};

    Vertex toVertex() const { return {vert_t(x), vert_t(y)}; }
};

using OffsetF = VertexF;

using VertexL = Clipper2Lib::Point64; // Clipper2Lib 是一个用于几何运算（尤其是多边形操作）的库。Clipper2Lib::Point64 是库中用于表示二维点的一个类型，采用 64 位整数表示坐标。

using Polygon = Clipper2Lib::Path64; // 按点存储的多边形

using PolygonId = size_t; // polygon offset in layout file or pattern poly index
// 多边形索引
using PatternId = std::vector<PolygonId>;

using PatternIds = std::vector<PatternId>;

struct BoundingBox
{
    vert_t lx{VERT_MAX}, ly{VERT_MAX}, rx{VERT_MIN}, uy{VERT_MIN};

    BoundingBox() = default;

    BoundingBox(vert_t lx, vert_t ly, vert_t rx, vert_t uy) : lx(lx), ly(ly), rx(rx), uy(uy) {}

    BoundingBox(VertexF center, double w, double h)
    {
        auto &[cx, cy] = center;
        lx = vert_t(cx - w / 2);
        ly = vert_t(cy - h / 2);
        rx = vert_t(cx + w / 2);
        uy = vert_t(cy + h / 2);
    }

    static BoundingBox infinite() { return {VERT_MIN, VERT_MIN, VERT_MAX, VERT_MAX}; }

    void update(Vertex vert)
    {
        lx = std::min(lx, vert.x);
        ly = std::min(ly, vert.y);
        rx = std::max(rx, vert.x);
        uy = std::max(uy, vert.y);
    }

    BoundingBox operator+(const BoundingBox &o) const // 取外返回一个新的Box
    {
        return {std::min(lx, o.lx), std::min(ly, o.ly), std::max(rx, o.rx), std::max(uy, o.uy)};
    }

    BoundingBox operator^(const BoundingBox &o) const // 取内返回一个新的Box
    {
        return {std::max(lx, o.lx), std::max(ly, o.ly), std::min(rx, o.rx), std::min(uy, o.uy)};
    }

    void operator+=(const BoundingBox &o) // 取外，更新当前对象
    {
        lx = std::min(lx, o.lx);
        ly = std::min(ly, o.ly);
        rx = std::max(rx, o.rx);
        uy = std::max(uy, o.uy);
    }

    BoundingBox operator-(Vertex v) const // 平移一个点位，返回一个新的Box
    {
        return {lx - v.x, ly - v.y, rx - v.x, uy - v.y};
    }

    BoundingBox operator+(Vertex v) const // 平移一个点位，返回一个新的Box
    {
        return {lx + v.x, ly + v.y, rx + v.x, uy + v.y};
    }

    bool operator==(const BoundingBox &o) const
    {
        return width() == o.width() && height() == o.height();
    }

    bool operator<=(const BoundingBox &o) const
    {
        return width() <= o.width() && height() <= o.height();
    }

    double cx() const { return double(lx + rx) / 2; }

    double cy() const { return double(ly + uy) / 2; }

    vert_t width() const { return rx - lx; }

    vert_t height() const { return uy - ly; }

    // strictly
    bool in(const BoundingBox &o) const { return o.lx < lx && rx < o.rx && o.ly < ly && uy < o.uy; }

    // loosely
    bool looseIn(const BoundingBox &o) const { return o.lx <= lx && rx <= o.rx && o.ly <= ly && uy <= o.uy; }

    // strictly
    bool contain(const Clipper2Lib::Point64 &v) const { return lx < v.x && v.x < rx && ly < v.y && v.y < uy; }

    // loosely
    bool looseContain(const Clipper2Lib::Point64 &v) const { return lx <= v.x && v.x <= rx && ly <= v.y && v.y <= uy; }

    // strictly
    bool contain(const Vertex &v) const { return lx < v.x && v.x < rx && ly < v.y && v.y < uy; }

    bool intersect(const BoundingBox &o) const { return o.lx < rx && lx < o.rx && o.ly < uy && ly < o.uy; } // 表示是否有交叉

    bool r90Equal(const BoundingBox &o) const
    {
        return height() == o.width() && width() == o.height();
    }

    bool strictEqual(const BoundingBox &o) const
    {
        return lx == o.lx && rx == o.rx && ly == o.ly && uy == o.uy;
    }

    BoundingBox r90XBox() const
    {
        return {ly, lx, uy, rx};
    }
};

enum MarkerType : uint8_t
{
    RAW = 0b00000001,
    X_AXIAL = 0b00000010,
    Y_AXIAL = 0b00000100,
    R180 = 0b00001000,
    R90CW = 0b00010000,
    R90CW_X_AXIAL = 0b00100000,
    R90CW_Y_AXIAL = 0b01000000,
    R90CW_R180 = 0b10000000,
    RECT_TYPE = RAW | X_AXIAL | Y_AXIAL | R180,
    R90CW_RECT_TYPE = R90CW | R90CW_X_AXIAL | R90CW_Y_AXIAL | R90CW_R180,
};

using Types = std::vector<MarkerType>;

using Marker = BoundingBox;

using Markers = std::vector<Marker>;

using Answer = Marker;

using Answers = std::vector<Answer>;

struct IndexBox : public BoundingBox
{
    PolygonId pid;

    explicit IndexBox(PolygonId pid, BoundingBox box = {}) : pid(pid), BoundingBox(box) {}
};

using IndexBoxes = std::vector<IndexBox>;

// 定义 Marker 的哈希函数
struct MarkerHasher
{
    std::size_t operator()(const Marker &box) const
    {
        return std::hash<vert_t>()(box.lx) ^ (std::hash<vert_t>()(box.ly) << 1) ^
               (std::hash<vert_t>()(box.rx) << 2) ^ (std::hash<vert_t>()(box.uy) << 3);
    }
};

// 定义 pair<Marker, MarkerType> 的哈希函数
struct PairHasher
{
    std::size_t operator()(const std::pair<Marker, MarkerType> &p) const
    {
        return MarkerHasher()(p.first) ^ (std::hash<int>()(static_cast<int>(p.second)) << 4);
    }
};

std::vector<std::pair<Marker, MarkerType>> findFrequentMarkers(const std::vector<std::pair<std::vector<Marker>, std::vector<MarkerType>>> &layers_result, int times)
{
    std::vector<std::pair<Marker, MarkerType>> result;
    if (times < 1)
        return result;
    // 用于统计每个 (Marker, MarkerType) 组合的次数
    std::unordered_map<std::pair<Marker, MarkerType>, int, PairHasher> count_map;

    // 遍历 layers_result 中每一层的数据
    for (const auto &layer : layers_result)
    {
        const auto &markers = layer.first;
        const auto &types = layer.second;

        // 假设每层的 markers 和 types 长度相同
        for (size_t i = 0; i < markers.size(); ++i)
        {
            count_map[{markers[i], types[i]}]++;
        }
    }

    // 收集出现次数大于等于 3 的 (Marker, MarkerType) 组合
    for (const auto &entry : count_map)
    {
        if (entry.second >= times)
        {
            result.push_back(entry.first);
        }
    }

    return result;
}

#endif // PATTERNMATCH_GEOMETRY_H
