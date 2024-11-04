#include "check.h"

Checker::Checker(std::vector<std::vector<PatternPoly>> &&multilayer_pattern_polys, Marker marker) : multilayer_pattern_polys_(std::move(multilayer_pattern_polys)), marker_(marker)
{
    const auto &poly2path = [&](const PatternPoly &poly)
    {
        Path64 ret;
        for (auto &[x, y] : poly)
        {
            ret.emplace_back(x, y);
        }
        return ret;
    };
    const auto &polys2Paths = [&](const std::vector<PatternPoly> &polys)
    {
        Paths64 ret;
        for (auto &poly : polys)
        {
            ret.push_back(poly2path(poly));
        }
        return ret;
    };
    const auto &modifyPaths = [&](Paths64 paths, MarkerType type)
    {
        auto mw = marker_.rx, mh = marker_.uy;
        const auto &modifyPaths = [&](const std::function<void(Point64 &)> &func)
        {
            for (auto &path : paths)
            {
                for (auto &p : path)
                {
                    func(p);
                }
            }
        };
        switch (type)
        {
        case RAW:
            break;
        case X_AXIAL:
            modifyPaths([&](Point64 &p)
                        { p.y = mh - p.y; });
            for (auto &path : paths)
                std::reverse(path.begin(), path.end());
            break;
        case Y_AXIAL:
            modifyPaths([&](Point64 &p)
                        { p.x = mw - p.x; });
            for (auto &path : paths)
                std::reverse(path.begin(), path.end());
            break;
        case R180:
            modifyPaths([&](Point64 &p)
                        { p = {mw - p.x, mh - p.y}; });
            break;
        case R90CW:
            modifyPaths([&](Point64 &p)
                        { p = {p.y, mw - p.x}; });
            break;
        case R90CW_X_AXIAL:
            modifyPaths([&](Point64 &p)
                        { p = {p.y, p.x}; });
            for (auto &path : paths)
                std::reverse(path.begin(), path.end());
            break;
        case R90CW_Y_AXIAL:
            modifyPaths([&](Point64 &p)
                        { p = {mh - p.y, mw - p.x}; });
            for (auto &path : paths)
                std::reverse(path.begin(), path.end());
            break;
        case R90CW_R180:
            modifyPaths([&](Point64 &p)
                        { p = {mh - p.y, p.x}; });
            break;
        default:
            break;
        }
        return paths;
    };
    const auto &getPatternString = [](Paths64 &paths)
    {
        const auto &getPathString = [](Path64 &path)
        {
            const auto &calcDirection = [](const Point64 &p)
            {
                auto &[x, y] = p;
                assert((x == 0) ^ (y == 0));
                if (y == 0 && x > 0)
                    return 'E';
                if (y == 0 && x < 0)
                    return 'W';
                if (y > 0 && x == 0)
                    return 'N';
                if (y < 0 && x == 0)
                    return 'S';
                return ' ';
            };
            std::string ret;
            size_t sz = path.size();
            for (int i = 0; i < sz; i++)
            {
                auto cur_d = calcDirection(path[(i + 1) % sz] - path[i % sz]);
                ret.push_back(cur_d);
            }
            return ret;
        };
        std::vector<std::string> ret;
        for (auto &path : paths)
        {
            auto path_str = getPathString(path);
            ret.push_back(path_str + path_str);
        }
        return ret;
    };

    init();
    num_layer = multilayer_pattern_polys_.size();

    for (size_t i = 0; i < num_layer; ++i)
    {
        auto &pattern_polys_ = multilayer_pattern_polys_[i];

        auto raw_paths = polys2Paths(pattern_polys_);
        // auto raw_tolerance = calcRawTolerance(pattern_polys_);
        for (int t = 0; t < 8; t++)
        {
            multilayer_target_patterns_[i].push_back(modifyPaths(raw_paths, MarkerType(1 << t)));
            // flex_tolerances_.push_back(modifyPaths(raw_tolerance, MarkerType(1 << t)));
            multilayer_pattern_strings_[i].push_back(getPatternString(multilayer_target_patterns_[i].back()));
        }
    }
}

void Checker::init()
{
    const auto &align = [&]()
    {
        Offset off({marker_.lx, marker_.ly});
        // Vertex 结构体没有定义任何构造函数，而 {} 列表初始化可以用来直接初始化成员变量，而 () 则无法在没有匹配构造函数的情况下直接初始化结构体。
        for (auto &pattern_polys_ : multilayer_pattern_polys_)
        {
            for (auto &pattern_poly : pattern_polys_)
            {
                for (auto &v : pattern_poly)
                {
                    v -= off;
                }
                pattern_poly.box = pattern_poly.box - off;
            }
        }
        marker_ = marker_ - off;
    };

    if (marker_.lx != 0 || marker_.ly != 0)
        align();

    for (std::vector<PatternPoly> pattern_polys_ : multilayer_pattern_polys_)
    {
        filter.push_back(MarkerFilter(pattern_polys_, marker_));
    }
}

std::vector<ptr<Filter>> Checker::getFilter()
{
    // 更新pattern_type的状态
    // 根据pattern_type的状态来选择filter的类型
    std::vector<ptr<Filter>> Filters;
    for (int i = 0; i < num_layer; i++)
    {
        // PatternPoly *inner_poly;
        int maxcount_inner_point = 0;
        int max_inner_point_index = 0;
        int maxcount_inner_poly = 0;
        int maxcount_inner_poly_index = 0;
        int index = 0;
        multilayer_pattern_type_[i] = INNER_NO_POINT_Layer;
        for (auto &poly : multilayer_pattern_polys_[i]) // 遍历一层的poly
        {
            int inner_point = 0;

            for (auto &point : poly)
            {
                if (marker_.contain(point))
                    inner_point++;
            }
            if (inner_point == poly.size())
            {
                if (inner_point > maxcount_inner_poly)
                {
                    maxcount_inner_poly = inner_point;
                    maxcount_inner_poly_index = index;
                    // inner_poly = &poly;
                }
                if (multilayer_pattern_type_[i] != INNER_POLY_Layer)
                    multilayer_pattern_type_[i] = INNER_POLY_Layer;
                multilayer_pattern_polys_type_[i][index] = INNER_POLY;
            }
            else if (inner_point > 1)
            {

                multilayer_pattern_polys_type_[i][index] = INNER_PATH_POLY;
                if (multilayer_pattern_type_[i] == INNER_POINT_Layer || multilayer_pattern_type_[i] == INNER_NO_POINT_Layer)
                    multilayer_pattern_type_[i] = INNER_PATH_Layer;
            }
            else if (inner_point == 1)
            {
                multilayer_pattern_polys_type_[i][index] = INNER_POINT_POLY;
                if (multilayer_pattern_type_[i] == INNER_NO_POINT_Layer)
                    multilayer_pattern_type_[i] = INNER_POINT_Layer;
            }
            else if (inner_point == 0)
            {
                multilayer_pattern_polys_type_[i][index] = INNER_NO_POINT_POLY;
            }
            if (inner_point > maxcount_inner_point)
                max_inner_point_index = index;
            index++;
        }

        Paths64 target_poly_8;
        std::vector<std::string> target_poly_string_8;

        if (multilayer_pattern_type_[i] == INNER_POLY_Layer)
        {
            for (int j = 0; j < 8; j++)
            {
                target_poly_8.push_back(multilayer_target_patterns_[i][j][maxcount_inner_poly_index]);
                target_poly_string_8.push_back(multilayer_pattern_strings_[i][j][maxcount_inner_poly_index]);
            }
            Filters.push_back(ptr<Filter>(new POLYFilter(target_poly_8, target_poly_string_8, marker_)));
        }
        else if (multilayer_pattern_type_[i] == INNER_PATH_Layer)
        {
            for (int j = 0; j < 8; j++)
            {
                target_poly_8.push_back(multilayer_target_patterns_[i][j][max_inner_point_index]);
                target_poly_string_8.push_back(multilayer_pattern_strings_[i][j][max_inner_point_index]);
            }
            Filters.push_back(ptr<Filter>(new PATHFilter(target_poly_8, target_poly_string_8, marker_)));
        }
        else if (multilayer_pattern_type_[i] == INNER_POINT_Layer)
        {
            for (int j = 0; j < 8; j++)
            {
                target_poly_8.push_back(multilayer_target_patterns_[i][j][max_inner_point_index]);
                target_poly_string_8.push_back(multilayer_pattern_strings_[i][j][max_inner_point_index]);
            }
            Filters.push_back(ptr<Filter>(new POINTFilter(target_poly_8, target_poly_string_8, marker_)));
        }
        else
        {
            Filters.push_back(ptr<Filter>(new NOPOINTFIler()));
        }
    }
    return Filters;
}

Paths64 Checker::check(const std::vector<Polygon *> &polys, Marker &marker, MarkerType type, int layer_label)
{
    using namespace Clipper2Lib;
    const auto &align = [](Paths64 &subjects, Rect64 &clip)
    {
        // 矩形的左上角平移到了坐标原点，subjects中多边形也进行相应的偏移
        auto ox = clip.left, oy = clip.top;
        clip.left -= ox;
        clip.right -= ox;
        clip.top -= oy;
        clip.bottom -= oy;
        for (auto &path : subjects)
        {
            for (auto &p : path)
            {
                p.x -= ox;
                p.y -= oy;
            }
        }
    };
    const auto &getPaths = [](const std::vector<Polygon *> &polys)
    {
        Paths64 ret;
        for (auto *poly : polys)
            ret.push_back(*poly);
        return ret;
    };
    const auto &box2Rect = [](BoundingBox box)
    {
        auto [lx, ly, rx, uy] = box;
        return Rect64(lx, ly, rx, uy);
    };
    const auto &type2idx = [&](int type)
    {
        int idx = 0;
        while (type >>= 1)
            idx++;
        return idx;
    };

    Paths64 subjects = getPaths(polys);
    auto clip = box2Rect(marker);
    align(subjects, clip);

    Paths64 potential_pattern = RectClip(clip, subjects); // 对subjects进行裁剪，取与clip重叠的部分

    auto idx = type2idx(type);
    Paths64 xor_res = Xor(multilayer_target_patterns_[layer_label - 1][idx], potential_pattern, FillRule::NonZero);
    return xor_res;
}

Paths64 Checker::final_check(const std::vector<Polygon> &polys, Marker &marker, MarkerType type, int layer_label)
{
    using namespace Clipper2Lib;
    const auto &align = [](Paths64 &subjects, Rect64 &clip)
    {
        // 矩形的左上角平移到了坐标原点，subjects中多边形也进行相应的偏移
        auto ox = clip.left, oy = clip.top;
        clip.left -= ox;
        clip.right -= ox;
        clip.top -= oy;
        clip.bottom -= oy;
        for (auto &path : subjects)
        {
            for (auto &p : path)
            {
                p.x -= ox;
                p.y -= oy;
            }
        }
    };
    const auto &box2Rect = [](BoundingBox box)
    {
        auto [lx, ly, rx, uy] = box;
        return Rect64(lx, ly, rx, uy);
    };
    const auto &type2idx = [&](int type)
    {
        int idx = 0;
        while (type >>= 1)
            idx++;
        return idx;
    };

    Paths64 subjects = polys;
    auto clip = box2Rect(marker);
    align(subjects, clip);

    Paths64 potential_pattern = RectClip(clip, subjects); // 对subjects进行裁剪，取与clip重叠的部分

    auto idx = type2idx(type);
    Paths64 xor_res = Xor(multilayer_target_patterns_[layer_label - 1][idx], potential_pattern, FillRule::NonZero);
    return xor_res;
}