#include <iostream>
#include "core/FileOperation.h"

int main(int argc, char *argv[])
{
    system("chcp 65001");
    system("cls");
    if (argc <= 3)
    {
        std::cout << "用法: spdconv [歌曲目录] [输出目录] [倍速]\n";
        return 1;
    }
    core::fs::path songs(argv[1]), dist(argv[2]);
    double speed = std::stod(argv[3]);
    auto paths = core::AnalysisDirectory(songs, ".osu");
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
        core::ExportFiles(dist, name + "x" + argv[3] + entry.extension().string(), lines);
    }
    std::cout << "所有文件已输出到: " << dist << std::endl;
    return 0;
}