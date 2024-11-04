#include "config.h"
// #include "Logger.h"

void Config::parseArgs(int argc, char *argv[])
{
    using ArgPairs = std::pair<std::string, std::string>;
    std::vector<ArgPairs> arg_pairs;
    assert(argc % 2 == 1 && "Invalid arguments");
    for (int i = 1; i < argc; i += 2)
    {
        arg_pairs.emplace_back(argv[i], argv[i + 1]);
    }
    for (auto &p : arg_pairs)
    {
        auto &type = p.first;
        auto &value = p.second;
        if (type == "-layout")
            layout_path = value;
        else if (type == "-lib")
            pattern_path = value;
        else if (type == "-output")
            output_path = value;
        // else if (type == "-answer") answer_path = value;
        // else if (type == "-test_dir") test_dir = "./" + value;
        // else if (type == "-log") logger = std::make_shared<Logger>(value);
        // else if (type == "-test" && value != "all") tests.push_back(value);
        // else if (type == "-test" && value == "all") test_all = true;
        else if (type == "-thread")
            num_threads = std::stoi(value);
        // else if (type == "-with_rot_mir") with_rot_mir = (value == "Y");
        // else if (type == "-perf") perf = (value == "Y");
        else
            assert(false && "Invalid arguments");
    }
    // if (!logger)
    //     logger = std::make_shared<Logger>();
}