#include "layout.h"
#include "read.h"

void Layout::cachePolygons(PatternIds &ptids_container, Reader &reader, int &layer_label)
{
    std::vector<PolygonId> pids;
    for (auto &ptids : ptids_container)
    {
        for (auto &pid : ptids)
        {
            pids.push_back(pid);
        }
    }
    std::sort(pids.begin(), pids.end()); // 升序排序
    pids.erase(std::unique(pids.begin(), pids.end()), pids.end());
    // std::unique 函数将相邻的重复元素移动到容器的末尾，并返回指向第一个重复元素的迭代器
    // 通过 pids.erase 删除从 unique 返回的迭代器到容器末尾的部分，即删除重复的 PolygonId。
    // pids 中只保留了不重复的、排序后的 PolygonId。
    polygons_cache_[layer_label] = reader.readPolygons(layout_file_, pids);
}

Polygon *Layout::getPolygon(PolygonId pid, int &layer_label)
{
    // 根据pid查找对应的Polygon
    // if (polygons_cache_.contains(pid))
    //     return &polygons_cache_.at(pid);
    // 在 C++ 标准库中，std::unordered_map 的 .contains 成员函数是在 C++20 中引入的。
    // 如果你使用的 C++ 标准低于 C++20（例如 C++11、C++14 或 C++17），那么 std::unordered_map 就不支持 .contains 方法
    auto it = polygons_cache_[layer_label].find(pid);
    if (it != polygons_cache_[layer_label].end())
        return &it->second;
    assert(false && "polygon not cache");
    return new Polygon;
}

std::vector<Polygon *> Layout::getPattern(std::vector<PolygonId> &pids, int &layer_label)
{
    std::vector<Polygon *> ret;
    for (auto id : pids)
    {
        ret.push_back(getPolygon(id, layer_label));
    }
    return ret;
}
