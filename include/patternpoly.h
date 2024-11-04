#ifndef PATTERNMATCH_PATTERNPOLY_H
#define PATTERNMATCH_PATTERNPOLY_H

#include "geometry.h"

#include "util.h"

class PatternPoly : public std::vector<Vertex> // 对象可以存边框和点
{
public:
    void push_back(const Vertex &vertex)
    {
        std::vector<Vertex>::push_back(vertex);
        updateBoundingBox(vertex);
    }

    void swap(PatternPoly &o)
    {
        std::vector<Vertex>::swap(o);
        std::swap(box, o.box);
    }
    // PatternPoly &newFlexPoly()
    // {
    //     flex_poly = new PatternPoly;
    //     return *flex_poly;
    // }

    // bool isFlexible() const { return flex_poly != nullptr; }

    // bool boxIsFlex() const { return isFlexible() && !box.strictEqual(flex_poly->box); }

    // bool llFix() const { return !isFlexible() || box.lx == flex_poly->box.lx && box.ly == flex_poly->box.ly; }

    // bool lrFix() const { return !isFlexible() || box.rx == flex_poly->box.rx && box.ly == flex_poly->box.ly; }

    // bool ulFix() const { return !isFlexible() || box.lx == flex_poly->box.lx && box.uy == flex_poly->box.uy; }

    // bool urFix() const { return !isFlexible() || box.rx == flex_poly->box.rx && box.uy == flex_poly->box.uy; }

    // bool hasFixVertex() const { return llFix() || lrFix() || ulFix() || urFix(); } // 边框中至少有一个角是不变的

    // BoundingBox minBox() const
    // { // 取重合部分的框
    //     if (isFlexible())
    //     {
    //         return box ^ flex_poly->box;
    //     }
    //     else
    //     {
    //         return box;
    //     }
    // }

    // BoundingBox maxBox() const
    // { // 取最小包含两个图形的框
    //     if (isFlexible())
    //     {
    //         return box + flex_poly->box;
    //     }
    //     else
    //     {
    //         return box;
    //     }
    //}

    BoundingBox box;
    // PatternPoly *flex_poly{}; // 可变的边框

private:
    void updateBoundingBox(Vertex vert)
    {
        box.lx = std::min(box.lx, vert.x);
        box.ly = std::min(box.ly, vert.y);
        box.rx = std::max(box.rx, vert.x);
        box.uy = std::max(box.uy, vert.y);
    }
};

#endif // PATTERNMATCH_PATTERNPOLY_H