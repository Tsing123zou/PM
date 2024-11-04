#include "config.h"
#include "read.h"
#include "layout.h"
#include "msqtree.h"

#include <unordered_set>
#include <sstream>
#include <regex>

using File = mio::mmap_source;

void flow()
{
    Reader read;
    auto checker = read.readPattern(g_config.pattern_path); // 读取一个pattern文件
    auto filter = checker.getFilter();                      // 得到该pattern的filter
    int num_layer = checker.get_layer_num();
    int NO_point_layer_count = 0;
    std::vector<std::pair<std::vector<Marker>, std::vector<MarkerType>>> layers_result;
    std::vector<MSQtreeManager> qms;
    Layout layout(g_config.layout_path, num_layer);
    File layout_file(g_config.layout_path);

    for (int i = 0; i < num_layer; i++)
    {
        std::pair<std::vector<Marker>, std::vector<MarkerType>> final_result;
        if (filter[i]->filterType() == "NOPOINTFIler")
        {
            NO_point_layer_count++;
            continue;
        }
        std::pair<std::vector<Marker>, std::vector<MarkerType>> potential_markers;
        int layer_label = 0;
        read.readLayout_layer(layout, layout_file, *filter[i], potential_markers, layer_label);
        assert(layer_label == i);
        MSQtreeManager qm(layout, potential_markers.first, layer_label);
        qms.push_back(qm);
        PatternIds query_result;
        for (auto &search_box : potential_markers.first)
        {
            query_result.push_back(qm.query(search_box));
        }
        layout.cachePolygons(query_result, read, layer_label); // 利用query_result得到具体的多边形存储到layout的cache中
        for (size_t i = 0; i < query_result.size(); i++)
        {
            auto &ptid = query_result[i]; // marker相交的poly
            if (ptid.empty())
                continue;
            auto &marker = potential_markers.first[i];
            auto &type = potential_markers.second[i];
            auto Xor = checker.check(layout.getPattern(ptid, layer_label), marker, type, layer_label);
            if (Xor.empty())
            {
                final_result.first.push_back(marker);
                final_result.second.push_back(type);
            }
        }
        layers_result.push_back(final_result);
    }
    std::vector<std::pair<Marker, MarkerType>> potentail_layer_result = findFrequentMarkers(layers_result, 3 - NO_point_layer_count);
    std::vector<std::pair<Marker, std::vector<Paths64>>> Xor_res;
    for (int i = 0; i < potentail_layer_result.size(); i++)
    {
        auto &one_result = potentail_layer_result[i];
        std::vector<Paths64> one_marker_res;
        int one_marker_xorempty_layer_count;
        // 这里有些层肯定是empty的，所以可以将potentail_layer_result的数据结构进行优化，对于前面已经判断过的直接跳过
        for (int j = 0; j < num_layer; j++)
        {
            PatternId query_one_marker_patternID = qms[j].query(one_result.first);
            // layout.insertcachePolygons(query_one_marker_patternID, read, j);
            std::vector<Polygon> query_one_marker_polygons = read.check_readPolygons(layout_file, query_one_marker_patternID);
            auto Xor = checker.final_check(query_one_marker_polygons, one_result.first, one_result.second, j);
            if (Xor.empty())
                one_marker_xorempty_layer_count++;
            one_marker_res.push_back(Xor);
        }
        if (one_marker_xorempty_layer_count >= 3 && one_marker_xorempty_layer_count < num_layer)
            Xor_res.push_back(std::pair(one_result.first, one_marker_res));
    }
}