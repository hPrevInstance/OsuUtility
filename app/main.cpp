#include <iostream>
#include "core/FileOperation.h"
#include "core/AudioProcess.h"

int main(int argc, char *argv[])
{
    if (argc <= 3)
    {
        std::cout << "用法: spdconv [歌曲目录] [输出目录] [倍速]" << std::endl;
        return 1;
    }
    std::cout << core::BuildCommand(argv[3], "", 44100, core::TEMPO);
    core::fs::path songs(argv[1]), dist(argv[2]);
    double speed = std::stod(argv[3]);
    auto paths = core::AnalysisDirectory(songs, {".osu"});
    for (const auto &entry : paths)
    {
        auto lines = core::LoadChartFile(entry);
        lines = core::ProcessChartFile(entry, lines, speed);
        auto name = entry.filename().string();
        auto point = name.find('.');
        if (point != std::string::npos)
        {
            name = name.substr(0, point);
        }
        core::ExportFiles(dist, fmt::format("{}{}{}{}", name, argv[3], "x", entry.filename().extension().string()), lines);
    }
    std::cout << "所有文件已输出到: " << dist << std::endl;
    return 0;
}