
#ifndef PATTERNMATCH_LAYOUT_H
#define PATTERNMATCH_LAYOUT_H
#include "geometry.h"
#include <set>
#include <map>
#include <vector>
#include <cstdlib>
#include <cstring>

#include "mio.hpp"
// #include "TContainer.h"
#include "container.h"
#include "util.h"
#include "read.h"

class Layout
{
    using File = mio::mmap_source;

    // mio::mmap_source 的主要功能
    // 文件读取：将文件的内容映射到内存中，允许用户像读内存一样读取文件内容。
    // 延迟加载：文件内容不会一次性加载到内存中，而是按需从磁盘加载（类似于分页内存管理），这意味着即使文件很大，也不会立即占用大量内存。
    // 自动管理文件生命周期：mmap_source 会根据文件的生命周期自动映射和取消映射文件，不必手动管理文件指针。

public:
    explicit Layout(const std::string &layout_path, size_t num_layer) : layout_file_(layout_path),
                                                                        layout_iboxes(num_layer),   // 使用 num_layer 初始化 iboxes_
                                                                        layout_box_(num_layer),     // 使用 num_layer 初始化 container_box_
                                                                        polygons_cache_(num_layer), // 使用 num_layer 初始化 polygons_cache_
                                                                        layer_poly_num(num_layer, 0)
    {
    }

    void cachePolygons(PatternIds &ptids_container, Reader &reader, int &layer_label);
    // void cachePolygons(TContainer<PatternIds> &ptids_container);

    // void cachePolygons();

    Polygon *getPolygon(PolygonId pid, int &layer_label); // using Polygon = Clipper2Lib::Path64;

    std::vector<Polygon *> getPattern(std::vector<PolygonId> &pids, int &layer_label);

    // TContainer<IndexBoxes> iboxes_;         // 记录layout中每个poly以及在文件中的cur_off
    // TContainer<BoundingBox> container_box_; // 对应iboxes的范围
    TContainer<BoundingBox> layout_box_; // 每层总的layout范围
    TContainer<IndexBoxes> layout_iboxes;
    TContainer<int> layer_poly_num;

private:
    File layout_file_;
    TContainer<std::unordered_map<PolygonId, Polygon>> polygons_cache_;
    // TContainer<std::unordered_map<PolygonId, Polygon>> polygons_cache_; // std::unordered_map<PolygonId, Polygon> 可以存储多个 <PolygonId, Polygon> 键值对，但每个键（即 PolygonId）在整个映射中必须是唯一的。
    // TContainer<PolygonId> cache_table_;                                 // 用于存储每个线程对应的缓存范围
};

class LayoutPoly : public std::vector<Vertex>
{
};

#endif // PATTERNMATCH_LAYOUT_H