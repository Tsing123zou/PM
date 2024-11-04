#ifndef PATTERNMATCH_CONFIG_H
#define PATTERNMATCH_CONFIG_H

#include <vector>
#include <string>
#include <filesystem>

// #include "types.h"

namespace fs = std::filesystem;

struct Logger;
struct Config
{
    std::string layout_path;
    std::string pattern_path;
    std::string output_path;
    std::string answer_path;
    // fs::path test_dir = "./test";
    // std::vector<std::string> tests;
    // sptr<Logger> logger;
    int num_threads = 1;
    // bool with_rot_mir = true;
    // bool test_all = false;
    // bool perf = false;

    void parseArgs(int argc, char *argv[]);
};

extern Config g_config; // 即便有了 extern 关键字，你仍然需要使用 g_config 的前缀来访问其中的成员变量

#endif // PATTERNMATCH_CONFIG_H