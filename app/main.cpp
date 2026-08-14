#include "CLI/CLIHandler.hpp"
#include "core/Error.hpp"
#include "gui/GUIHandler.hpp"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
    /**
     * @brief 尽力开启控制台 UTF-8 输出与 ANSI 颜色支持
     */
    void SetupConsole()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE)
        {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode))
            {
                SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }
#endif
    }
} // namespace

int main(int argc, char **argv)
{
    SetupConsole();
    std::ios::sync_with_stdio(false);

    if (argc == 1)
    {
        return RunGUI(argc, argv);
    }

    // CLI 顶层：统一捕获未在内部处理的异常并返回对应退出码
    try
    {
        return RunCLI(argc, argv);
    }
    catch (const core::OspError &e)
    {
        std::cerr << "[错误] " << e.what() << std::endl;
        switch (e.Kind())
        {
        case core::ErrorKind::Parameter:
            return core::EXIT_USAGE;
        case core::ErrorKind::ExternalProcess:
            return core::EXIT_EXTERNAL;
        default:
            return core::EXIT_PARTIAL_FAILURE;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[错误] 未预期的异常：" << e.what() << std::endl;
        return core::EXIT_PARTIAL_FAILURE;
    }
}