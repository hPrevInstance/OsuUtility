#pragma once

#include "CLI11.hpp"
#include "tools/ProcessToolsBase.hpp"

int RunCLI(int argc, char **argv)
try
{
    CLI::App app("OSU!Mania!谱面倍速调节器", "OSUspeeder");

    std::vector<core::fs::path> files;
    app.add_option("-i,--input", files, "要转换的文件列表或目录。对于每个谱面或音频文件，都对其进行转换操作；对于目录则遍历其中每个文件并进行转换")
        ->required()
        ->check(CLI::ExistingPath)
        ->type_name("PATH...");

    std::string pitch, tempo;
    auto speed = app.add_option_group("speed", "为所有谱面设置同一速度，推荐使用分数使结果更精确");
    speed->require_option(1);
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

    auto process = [&](const core::fs::path &entry) -> void
    {
        if (!core::fs::is_regular_file(entry) || entry.extension() != ".osu")
        {
            return;
        }

        tools::BeatmapTaskBase beatmap(core::fs::absolute(entry));

        auto dp = pitch;
        auto dt = tempo;

        if (diff)
        {
            std::cout << "当前文件：" << entry.filename() << std::endl;
            std::cout << "输入变速，按回车跳过：";
            std::getline(std::cin, dt);
            std::cout << std::endl;
            std::cout << "输入变调，按回车跳过：";
            std::getline(std::cin, dp);

            dp = tools::trim(dp);
            dt = tools::trim(dt);

            if (dp.empty() && dt.empty())
            {
                std::cerr << "错误：必须至少提供变速或变调之一，跳过当前文件\n";
                return;
            }
        }

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
    };

    for (const auto &path : files)
    {

        if (core::fs::is_regular_file(path))
        {
            process(path);
        }
        else if (core::fs::is_directory(path))
        {
            if (recursive)
            {
                for (const auto &entry : core::fs::recursive_directory_iterator(path))
                {
                    process(entry);
                }
            }
            else
            {
                for (const auto &entry : core::fs::directory_iterator(path))
                {
                    process(entry);
                }
            }
        }
        else
        {
            std::cerr << "不支持该文件类型：" + path.filename().string();
            return 1;
        }
    }
    return 0;
}
catch (const std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}