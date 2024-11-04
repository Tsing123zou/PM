#include "config.h"

Config g_config;

void flow();

int main(int argc, char *argv[])
{
    g_config.parseArgs(argc, argv);

    // if (g_config.tests.empty() && !g_config.test_all)
    // {
    //     flow();
    // }
    // else
    // {
    //     auto framework = buildTestFramework(); // 得到整个框架的对象
    //     if (g_config.test_all)
    //         framework.runAll();
    //     else
    //         framework.runTests(g_config.tests);
    // }

    return 0;
}