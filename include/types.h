#ifndef PATTERNMATCH_TYPES_H
#define PATTERNMATCH_TYPES_H

#include <limits>

using uint = unsigned int;
using vert_t = int;
using index_t = uint;

template <typename T>
struct RangeT
{ // 用于表示某种类型 T 的最小值和最大值。主要功能是检查某个值是否落在这个范围内，并定义了一个表示无限范围的静态方法。
    T min_{}, max_{};

    inline bool contain(T n) const { return min_ <= n && n <= max_; }

    static RangeT infinite() { return {std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}; }
};

using Range = RangeT<int>;

const vert_t VERT_MAX = std::numeric_limits<vert_t>::max();
const vert_t VERT_MIN = std::numeric_limits<vert_t>::min();

template <class T>
using ptr = std::unique_ptr<T>;
template <class T>
using sptr = std::shared_ptr<T>;

#endif // PATTERNMATCH_TYPES_H
