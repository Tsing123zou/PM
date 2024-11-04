#include "read.h"

Checker Reader::readPattern(const std::string &pattern_path)
{
    //  每次read是读一个pattern，分成了许多层。
    File file(pattern_path);
    // size_t cur_off = 0;//由于pattern都在一个文件下面，所以要记录cur_off的位置

    const auto &getChar = [&]()
    { return file[pattern_cur_off++]; };

    const auto &checkpattern = [&]()
    {
        const char *str = "pattern";
        assert(strncmp(str, (const char *)&file[pattern_cur_off], 7) == 0);
        pattern_cur_off += 7;
        while (getChar() != '\n')
            ;
    };

    const auto &checklayer = [&]()
    {
        const char *str = "layer";
        assert(strncmp(str, (const char *)&file[pattern_cur_off], 5) == 0);
        pattern_cur_off += 5;
        int label_layer = 0;
        while (1)
        {
            char a = getChar();
            if (a != '\n')
                label_layer = 10 * label_layer + a - '0';
            else
                break;
        }
        return label_layer;
    };

    const auto &checkTitle = [&](const char *str, int len)
    {
        assert(strncmp(str, (const char *)&file[pattern_cur_off], len) == 0);
        pattern_cur_off += len;
    };

    const auto &readPolygon = [&](PatternPoly &ret)
    {
        for (char end = ','; end == ','; end = getChar())
        {
            auto tv = readVertex(file, pattern_cur_off);
            ret.push_back(tv);
            assert(end == ',' || end == '\n');
        }
    };
    // const auto &readFlexPoly = [&](PatternPoly &ret)
    // {
    //     getChar();
    //     readPolygon(ret);
    //     getChar();
    // };
    const auto &readMarker = [&]()
    {
        Marker marker{};
        for (char end = ','; end == ','; end = getChar())
        {
            auto tv = readVertex(file, pattern_cur_off);
            marker.update(tv);
            assert(end == ',' || end == '\n');
        }
        return marker;
    };
    // checkTitle("pattern:\n", 9);
    //  修改读取情况
    //  去除[]
    //  加上layer:
    //  每次read是读一个pattern，分成了许多层。
    checkpattern();
    std::vector<std::vector<PatternPoly>> multilayer_pattern_polys;
    int now_layer = 1;
    while (file[pattern_cur_off] == 'l')
    {
        std::vector<PatternPoly> pattern_polys;
        int layer_diff = checklayer() - now_layer;
        if (layer_diff == 0)
            now_layer++;
        else
        {
            now_layer += layer_diff;
            while (layer_diff > 0)
            {
                multilayer_pattern_polys.push_back(pattern_polys);
                layer_diff--;
            }
        }
        for (char tc = file[pattern_cur_off]; tc == '('; tc = file[pattern_cur_off])
        {
            if (tc == '(')
                readPolygon(pattern_polys.emplace_back());
            // if (tc == '[')
            //     readFlexPoly(pattern_polys.back().newFlexPoly());
        }
        multilayer_pattern_polys.push_back(pattern_polys);
    }
    checkTitle("marker:\n", 8);
    auto marker = readMarker();

    return {std::move(multilayer_pattern_polys), marker};
}

Vertex Reader::readVertex(const File &file, size_t &cur_off)
{
    const auto &getChar = [&]()
    { return file[cur_off++]; };
    const auto &readInteger = [&]()
    {
        // read "x," or "x)"
        vert_t ret = 0;
        int flag = 1;
        char tc = getChar();
        if (tc == '-')
        {
            flag = -1;
            tc = getChar();
        }
        do
        {
            ret *= 10;
            ret += tc - '0';
        } while (tc = getChar(), '0' <= tc && tc <= '9');
        return flag * ret;
    };

    // read "(x,x)"
    auto c = getChar();
    assert(c == '(');
    vert_t x = readInteger();
    vert_t y = readInteger();
    return Vertex{x, y};
}

void Reader::readLayout_layer(Layout &layout, File &file, Filter &filter, std::pair<std::vector<Marker>, std::vector<MarkerType>> potential_markers, int &layer_label)
{
    // File file(layout_path);
    // Layout layout(layout_path);

    const auto &getChar = [&]()
    { return file[layout_cur_off++]; };

    const auto &checklayer = [&]()
    {
        const char *str = "layer";
        assert(strncmp(str, (const char *)&file[layout_cur_off], 5) == 0);
        layout_cur_off += 5;
        int label_layer = 0;
        while (1)
        {
            char a = getChar();
            if (a != '\n')
                label_layer = 10 * label_layer + a - '0';
            else
                break;
        }
        return label_layer;
    };

    const auto &readNullVertex = [&]()
    {
        while (getChar() != ',')
            ;
        while (getChar() != ')')
            ;
    };
    const auto &readPolygon = [&](LayoutPoly &ret, int &layer_label)
    {
        // 读取一个多边形得到它的box
        IndexBox box(layout_cur_off); // 存的是在文件中的偏移

        for (char end = ','; end == ','; end = getChar())
        {
            auto tv = readVertex(file, layout_cur_off);
            box.update(tv);
            ret.push_back(tv);
            // readNullVertex(); // 只更新边框的话，不需要相邻的点
            assert(end == ',' || end == '\n');
        }
        layout.layout_iboxes[layer_label].push_back(box); // 存读到的box，这里仅仅读box
        layout.layer_poly_num[layer_label]++;
    };
    layer_label = checklayer();
    for (char tc = file[layout_cur_off]; tc == '('; tc = file[layout_cur_off])
    {
        LayoutPoly ret;
        readPolygon(ret, layer_label);
        std::pair<std::vector<Marker>, std::vector<MarkerType>> onepoly_potential_markers = filter.getPotentialMarkers(ret);
        potential_markers.first.insert(potential_markers.first.end(), onepoly_potential_markers.first.begin(), onepoly_potential_markers.first.end());
        potential_markers.second.insert(potential_markers.second.end(), onepoly_potential_markers.second.begin(), onepoly_potential_markers.second.end());
    }
    // return layout;
}

std::unordered_map<PolygonId, Polygon> Reader::readPolygons(const File &layout_file, std::vector<PolygonId> &pids)
{
    size_t cur_off = 0;

    const auto &setFilePointer = [&](size_t offset)
    { cur_off = offset; };
    const auto &getChar = [&]()
    { return layout_file[cur_off++]; };
    const auto &readPolygon = [&]()
    {
        Polygon ret;
        for (char end = ','; end == ','; end = getChar())
        {
            auto [x, y] = readVertex(layout_file, cur_off);
            ret.emplace_back(x, y);
            assert(end == ',' || end == '\n');
        }
        return ret;
    };

    std::unordered_map<PolygonId, Polygon> ret;
    for (auto id : pids)
    {
        setFilePointer(id);
        ret.emplace(id, readPolygon());
        // 将读取到的多边形数据添加到 ret 的哈希表中，其中键是 id，值是 readPolygon() 返回的 Polygon。
    }
    return ret;
}

std::vector<Polygon> Reader::check_readPolygons(const File &layout_file, std::vector<PolygonId> &query_one_marker_patternID)
{
    size_t cur_off = 0;

    const auto &setFilePointer = [&](size_t offset)
    { cur_off = offset; };
    const auto &getChar = [&]()
    { return layout_file[cur_off++]; };
    const auto &readPolygon = [&]()
    {
        Polygon ret;
        for (char end = ','; end == ','; end = getChar())
        {
            auto [x, y] = readVertex(layout_file, cur_off);
            ret.emplace_back(x, y);
            assert(end == ',' || end == '\n');
        }
        return ret;
    };
    std::vector<Polygon> query_one_marker_polygons;
    for (auto id : query_one_marker_patternID)
    {
        setFilePointer(id);
        query_one_marker_polygons.push_back(readPolygon());
    }
    return query_one_marker_polygons;
}