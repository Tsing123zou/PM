#ifndef PATTERNMATCH_TCONTAINER_H
#define PATTERNMATCH_TCONTAINER_H
#include <vector>

// #include "Config.h"

// for multi thread relative container
template <class Content>
struct TContainer : public std::vector<Content>
{
    using Super = std::vector<Content>;
    // TContainer() : Super(g_config.num_threads) {}
    //  表示要给容器初始化的大小。这个值用来初始化 std::vector，使其创建时有 g_config.num_threads 个元素。
    TContainer(size_t num_layer) : Super(num_layer) {}
    TContainer(size_t size, const Content &value) : Super(size, value) {}
};

#endif // PATTERNMATCH_TCONTAINER_H
