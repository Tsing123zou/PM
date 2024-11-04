#ifndef PATTERNMATCH_CHECKER_H
#define PATTERNMATCH_CHECKER_H

#include <vector>

#include "clipper.h"

#include "geometry.h"
#include "patternpoly.h"
#include "filter.h"
#include "util.h"

using Paths64 = Clipper2Lib::Paths64;
using Path64 = Clipper2Lib::Path64;
using Point64 = Clipper2Lib::Point64;
using Strings = std::vector<std::string>;
using Polygon = Clipper2Lib::Path64; // 按点存储的多边形
// Ponit64包含了两个元素，T x，T y；
// Path64是结构体Point64的向量
// Paths64是向量Path64的向量

class Checker
{
    struct MarkerFilter
    {
        std::vector<std::vector<Vertex>> fix_vs_{};      // 二维向量，每个元素是一组点
        std::vector<std::pair<Vertex, Offset>> fix_v_{}; // 一维向量，每个元素是一条边
        Marker marker_;                                  // Marker=Boundingbox

        MarkerFilter() = default;

        MarkerFilter(std::vector<PatternPoly> &pattern_polys, Marker marker);

        inline bool operator()(const std::vector<Polygon *> &polys, Marker &marker, MarkerType type) const;
        // 这是一个函数调用运算符重载，用于对输入的多边形 polys、marker 和类型 type 进行某种检查和过滤。
    };
    enum Pattern_Layer_Poly_Type
    {
        INNER_POLY,
        INNER_PATH_POLY,
        INNER_POINT_POLY,
        INNER_NO_POINT_POLY
    };

    enum Pattern_Layer_Type
    {
        INNER_POLY_Layer,
        INNER_PATH_Layer,
        INNER_POINT_Layer,
        INNER_NO_POINT_Layer
    };

public:
    Checker(std::vector<std::vector<PatternPoly>> &&multilayer_pattern_polys, Marker marker); // 右值引用

    void init();

    std::vector<ptr<Filter>> Checker::getFilter();

    // Range getInnerCount();

    Paths64 Checker::check(const std::vector<Polygon *> &polys, Marker &marker, MarkerType type, int layer_label);
    Paths64 Checker::final_check(const std::vector<Polygon> &polys, Marker &marker, MarkerType type, int layer_label);

    // bool finalCheck(Paths64 &pattern, int tpi);

    std::vector<std::vector<PatternPoly>> get_multilayer_PatternPolys() { return multilayer_pattern_polys_; }

    std::vector<PatternPoly> get_onelayer_PatternPolys(int i) { return multilayer_pattern_polys_[i]; }

    int get_layer_num()
    {
        return num_layer;
    }

    Marker getMarker()
    {
        return marker_;
    }

private:
    std::vector<std::vector<Pattern_Layer_Poly_Type>> multilayer_pattern_polys_type_;
    std::vector<Pattern_Layer_Type> multilayer_pattern_type_;

    std::vector<std::vector<PatternPoly>> multilayer_pattern_polys_;
    // 存一个pattern的多边形，一个元素表示一层的polys
    std::vector<std::vector<Paths64>> multilayer_target_patterns_;
    // 存一个pattern多边形转变成Paths64形式，这里vector包含了八个元素
    std::vector<std::vector<Strings>> multilayer_pattern_strings_;
    // 存target_patterns_图形从第一个点开始的走向，这里vector包含了八个元素，每个元素包含了多边形数量的字符串，表示每个图形从第一个点开始的走向
    // using Strings = std::vector<std::string>;
    std::vector<MarkerFilter> filter;
    Marker marker_;
    int num_layer;
};

#endif // PATTERNMATCH_CHECKER_H
