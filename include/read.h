#ifndef PATTERNMATCH_READER_H
#define PATTERNMATCH_READER_H

#include <vector>
#include <functional>
#include <mutex>

#include "mio.hpp"
#include "layout.h"
#include "filter.h"
#include "check.h"
// #include "Thread.h"

class Reader
{
    using File = mio::mmap_source;

public:
    size_t pattern_cur_off = 0;
    size_t layout_cur_off = 0;
    Checker readPattern(const std::string &pattern_path); // 读入模板文件，取消静态函数，因为cur_off会改变
    // static Checker readPattern(const std::string &pattern_path); // 读入模板文件

    void readLayout_layer(Layout &layout, File &file, Filter &filter, std::pair<std::vector<Marker>, std::vector<MarkerType>> potential_markers, int &layer_label); // 读入版图文件
    //  static Layout readLayout(const std::string &layout_path, Filter &filter); // 读入版图文件

    std::unordered_map<PolygonId, Polygon> readPolygons(const File &layout_file, std::vector<PolygonId> &pids);
    // static std::unordered_map<PolygonId, Polygon> readPolygons(const File &layout_file, std::vector<PolygonId> &pids);
    std::vector<Polygon> check_readPolygons(const File &layout_file, std::vector<PolygonId> &query_one_marker_patternID);

    // static std::vector<BoundingBox> readMarkers(const std::string &file_path);

private:
    inline Vertex readVertex(const File &file, size_t &cur_off);

    // struct Slice
    // {
    //     size_t begin, size;
    // };
};

#endif // PATTERNMATCH_READER_H
