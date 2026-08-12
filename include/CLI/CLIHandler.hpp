/**
 * @file    CLIHandler.hpp
 * @brief   CLI入口
 *
 * 提供命令行界面的启动函数。当用户带命令行参数运行程序时，
 * 将由 main 调用本文件的 RunCLI 进入命令行模式。
 *
 * 主要流程：
 *  1. 用 CLI11 解析命令行选项
 *  2. 收集待处理的谱面文件列表
 *  3. 逐一对每个 .osu 谱面调用 BeatmapTaskBase 完成倍速处理
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup cli
 */

#pragma once

#include "CLI11.hpp"
#include "tools/BeatmapTaskBase.hpp"
#include <set>

/**
 * @brief 启动 CLI 界面并处理命令行
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 程序退出码
 */
int RunCLI(int argc, char **argv)
{
    CLI::App app("OSU!Mania!谱面倍速调节器", "osp");

    std::vector<core::fs::path> files;
    app.add_option("-i,--input", files, "要转换的文件列表或目录。对于每个谱面或音频文件，都对其进行转换操作；对于目录则遍历其中每个文件并进行转换")
        ->required()
        ->check(CLI::ExistingPath)
        ->type_name("PATH...");

    std::string pitch, tempo;
    auto speed = app.add_option_group("speed", "为所有谱面设置同一速度，推荐使用分数使结果更精确");
    auto p = speed->add_option("-p,--pitch", pitch, "设置变调，支持小数和分数");
    auto t = speed->add_option("-t,--tempo", tempo, "设置变速，支持小数和分数");

    bool diff = false;
    app.add_flag("-d,--diff", diff, "分别为每个谱面设置不同速度，推荐使用分数使结果更精确")->excludes(p, t);

    core::fs::path output;
    app.add_option("-o,--output", output, "输出目录，输入的所有文件和目录处理后都将包含在该目录中")
        ->required()
        ->type_name("DIR");

    bool recursive = false;
    app.add_flag("-r", recursive, "指定是否递归搜索所有目录，否则最多处理至第二层（第一层为虚拟根节点）");

    CLI11_PARSE(app, argc, argv);

    // 收集所有待处理的路径
    std::set<core::fs::path> paths;
    auto absOutput = core::fs::absolute(output);
    for (const auto &path : files)
    {
        if (core::fs::is_regular_file(path)) // 是普通文件则直接加入
        {
            if (core::fs::absolute(path).string().find(absOutput.string()) != 0)
            {
                paths.insert(path);
            }
        }
        else if (core::fs::is_directory(path)) // 是目录则遍历其中文件
        {
            if (core::fs::absolute(path) == absOutput)
            {
                continue;
            }
            if (recursive) // 递归遍历所有子目录
            {
                for (const auto &entry : core::fs::recursive_directory_iterator(path))
                {
                    auto absEntry = core::fs::absolute(entry.path());
                    if (absEntry.string().find(absOutput.string()) == 0)
                    {
                        continue;
                    }
                    paths.insert(entry.path());
                }
            }
            else
            {
                for (const auto &entry : core::fs::directory_iterator(path))
                {
                    auto absEntry = core::fs::absolute(entry.path());
                    if (absEntry.string().find(absOutput.string()) == 0)
                    {
                        continue;
                    }
                    paths.insert(entry.path());
                }
            }
        }
    }

    // 逐一对每个 .osu 谱面执行倍速处理
    for (const auto &path : paths)
    {
        try
        {
            // 仅处理常规文件且扩展名为 .osu 的谱面
            if (!core::fs::is_regular_file(path) || path.extension() != ".osu")
            {
                continue;
            }

            std::cout << "当前文件：" << path.filename().string() << std::endl;
            tools::BeatmapTaskBase beatmap(core::fs::absolute(path));

            auto dp = pitch; // 变调参数
            auto dt = tempo; // 变速参数

            // -d 模式下逐文件询问各谱面的速度
            if (diff)
            {
                std::cout << "输入变速，按回车跳过：";
                std::getline(std::cin, dt);
                std::cout << "输入变调，按回车跳过：";
                std::getline(std::cin, dp);

                dp = tools::trim(dp);
                dt = tools::trim(dt);

                // 两者都为空时默认不改变速度
                if (dp.empty() && dt.empty())
                {
                    dp = dt = "1";
                }
            }

            // 根据是否提供变调/变速参数组合出处理模式
            core::Option opt = 0;
            if (!dp.empty())
            {
                opt = opt | core::PITCH;
            }
            if (!dt.empty())
            {
                opt = opt | core::TEMPO;
            }

            beatmap.SetMode(opt);
            beatmap.SetPitch(dp);
            beatmap.SetTempo(dt);

            beatmap.Parse(output);

            std::cout << "处理完成！" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << "跳过当前文件\n"
                      << std::endl;
            continue;
        }
    }

    return 0;
}