/**
 * @file    CLIHandler.cpp
 * @brief   CLI 顶层分发实现
 * @ingroup cli
 */

#include "cli/CLIHandler.hpp"
#include "cli/SpeedCommand.hpp"
#include "cli/CLI11.hpp"
#include "core/Error.hpp"

#include <iostream>

int RunCLI(int argc, char **argv)
{
    using namespace cli;

    CLI::App app("OsuUtility —— osu! 工具集", kProgramName);
    app.set_version_flag("-V,--version", std::string(kProgramName) + " " + kVersion);

    //  注册功能子命令（后续新增功能在此追加注册）
    SpeedCommand speed;
    speed.Register(app);

    //  解析命令行
    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp &e)
    {
        std::cout << app.help();
        return core::EXIT_OK;
    }
    catch (const CLI::CallForAllHelp &e)
    {
        std::cout << app.help("", CLI::AppFormatMode::All);
        return core::EXIT_OK;
    }
    catch (const CLI::CallForVersion &e)
    {
        std::cout << e.what() << std::endl;
        return core::EXIT_OK;
    }
    catch (const CLI::ParseError &e)
    {
        std::cerr << Red("[错误] ") << e.what() << std::endl;
        std::cerr << Dim("使用 --help 查看用法。") << std::endl;
        return core::EXIT_USAGE;
    }

    //  分发到被选中的子命令
    if (speed.Selected())
    {
        return speed.Run();
    }

    //  未指定任何子命令，显示帮助
    std::cout << app.help() << std::endl;
    return core::EXIT_USAGE;
}
