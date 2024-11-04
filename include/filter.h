#ifndef PATTERNMATCH_FILTER_H
#define PATTERNMATCH_FILTER_H

#include <vector>
#include <unordered_map>
#include <fstream>

#include "config.h"
// #include "TContainer.h"
#include "geometry.h"
#include "patternpoly.h"
// #include "MSQtree.h"
// #include "Thread.h"
#include "util.h"
#include "check.h"
#include "layout.h"

#define TYPE(type) \
    std::string filterType() const override { return #type; }

class Filter
{
protected:
    using TargetPosF = OffsetF;
    using TargetType = MarkerType;
    using PosFTypePair = std::pair<TargetPosF, TargetType>;
    using PosFTypePairs = std::vector<PosFTypePair>;
    using TargetPos = Offset;
    using PosTypePair = std::pair<TargetPos, TargetType>;
    using PosTypePairs = std::vector<PosTypePair>;

    struct BoundingBoxF
    {
        double lx, ly, rx, uy;

        // BoundingBoxF + OffsetF = BoundingBox can be guaranteed
        BoundingBox operator+(OffsetF o) const
        { // 使用 OffsetF 的值对 BoundingBoxF 的坐标进行平移。ver_t是int
            return {vert_t(lx + o.x), vert_t(ly + o.y), vert_t(rx + o.x), vert_t(uy + o.y)};
        }
    };

    struct MarkerAttr
    {
        double ox, oy;
        vert_t w, h;
    };

public:
    // virtual void operator()(const BoundingBox &box, index_t ci) = 0;

    // virtual std::pair<TContainer<Markers>, TContainer<Types>> getPotentialMarkers() = 0;
    virtual std::pair<std::vector<Marker>, std::vector<MarkerType>> getPotentialMarkers(LayoutPoly &ploy) = 0;

    virtual std::string filterType() const = 0;

    virtual ~Filter() = default;
};

class NOPOINTFIler : public Filter
{
    TYPE(NOPOINTFIler)
    std::pair<std::vector<Marker>, std::vector<MarkerType>> getPotentialMarkers(LayoutPoly &ploy) override {}

    // void operator()(const BoundingBox &box, index_t ci) override {}
};

class POLYFilter : public Filter
{
public:
    TYPE(POLYFilter)
    POLYFilter(Paths64 &target_poly_8, std::vector<std::string> &target_poly_string_8, Marker marker) : target_poly_8_(target_poly_8), target_poly_string_8_(target_poly_string_8), marker_(marker_)
    {
        // const auto &getEdgeLength = [&](const Point64 &start, const Point64 &end)
        // {
        //     return std::abs(end.x - start.x) + std::abs(end.y - start.y);
        // };
        // point_count = target_poly_8_[0].size();
        // for (size_t i = 0; i < point_count; ++i)
        // {
        //     int length = getEdgeLength(target_poly_8_[0][i], target_poly_8_[0][(i + 1) % point_count]);
        //     targetLengths_ACW.push_back(length);
        // }
        // std::reverse_copy(targetLengths_ACW.begin(), targetLengths_ACW.end(), std::back_inserter(targetLengths_CW));
        const auto &getEdgeLength = [&](const Point64 &start, const Point64 &end)
        {
            return end.x - start.x + end.y - start.y;
        };
        const auto &getHead_type = [&](const Point64 &first, const Point64 &second)
        {
            return second.y - first.y;
        };
        bool head_type = getHead_type(target_poly_8[0][0], target_poly_8[0][1]);
        if (head_type)
            Head_types = {1, 0, 0, 1, 0, 1, 1, 0};
        else
            Head_types = {0, 1, 1, 0, 1, 0, 0, 1};
        std::vector<int> targetLengths_ACW;
        std::vector<int> targetLengths_CW;
        for (size_t i = 0; i < point_count; ++i)
        {
            int length = getEdgeLength(target_poly_8_[0][i], target_poly_8_[0][(i + 1) % point_count]);
            targetLengths_ACW.push_back(length);
        }
        std::reverse_copy(targetLengths_ACW.begin(), targetLengths_ACW.end(), std::back_inserter(targetLengths_CW));
        const auto &getfeature = [&](const bool x_, const bool y_, std::vector<int> feature)
        {
            std::vector<int> MIR_feature;
            if (x_ && y_)
            {
                for (int i = 0; i < feature.size(); i++)
                {
                    MIR_feature.push_back(-feature[i]);
                }
            }
            else if (x_ && !y_)
            {
                for (int i = 0; i < feature.size(); i += 2)
                {
                    MIR_feature.push_back(-feature[i]);
                }
            }
            else if (!x_ && y_)
            {
                for (int i = 1; i < feature.size(); i += 2)
                {
                    MIR_feature.push_back(-feature[i]);
                }
            }
            return MIR_feature;
        };
        features.push_back(targetLengths_ACW);
        features.push_back(getfeature(false, true, targetLengths_CW));
        features.push_back(getfeature(true, false, targetLengths_CW));
        features.push_back(getfeature(true, true, targetLengths_ACW));
        features.push_back(getfeature(true, false, targetLengths_ACW));
        features.push_back(getfeature(true, true, targetLengths_CW));
        features.push_back(targetLengths_CW);
        features.push_back(getfeature(true, false, targetLengths_CW));
    }
    std::pair<std::vector<Marker>, std::vector<MarkerType>> getPotentialMarkers(LayoutPoly &ploy);

private:
    std::vector<bool> Head_types;
    std::vector<std::vector<int>> features;
    int point_count;
    Paths64 target_poly_8_;
    // std::vector<int> targetLengths_ACW;
    // std::vector<int> targetLengths_CW;
    std::vector<std::string> target_poly_string_8_;
    Marker marker_;
    std::vector<std::vector<int>> nexts;
    void getNext()
    {
        for (int i = 0; i < 8; i++)
        {
            std::vector<int> next;
            int j = 0, t = -1;
            next.push_back(-1);
            while (j < int(features[i].size() - 1))
            {
                if (t == -1 || features[i][j] == features[i][t])
                {
                    j++;
                    t++;
                    if (features[i][j] == features[i][t])
                        next.push_back(next[t]);
                    else
                        next.push_back(t);
                }
                else
                {
                    t = next[t];
                }
            }
            nexts.push_back(next);
        }
    }
};

class PATHFilter : public Filter
{
public:
    TYPE(PATHFilter)
    PATHFilter(Paths64 &target_poly_8, std::vector<std::string> &target_poly_string_8, Marker marker) : target_poly_8_(target_poly_8), target_poly_string_8_(target_poly_string_8), marker_(marker_)
    {
        }
    std::pair<std::vector<Marker>, std::vector<MarkerType>> getPotentialMarkers(LayoutPoly &ploy);

private:
    Paths64 target_poly_8_;
    std::vector<std::string> target_poly_string_8_;
    Marker marker_;
};

class POINTFilter : public Filter
{
public:
    TYPE(POINTFilter)
    POINTFilter(Paths64 &target_poly_8, std::vector<std::string> &target_poly_string_8, Marker marker) : target_poly_8_(target_poly_8), target_poly_string_8_(target_poly_string_8), marker_(marker_)
    {
    }
    std::pair<std::vector<Marker>, std::vector<MarkerType>> getPotentialMarkers(LayoutPoly &ploy);

private:
    Paths64 target_poly_8_;
    std::vector<std::string> target_poly_string_8_;
    Marker marker_;
};

#undef TYPE

#endif // PATTERNMATCH_FILTER_H