/**
 * @file    FileOperation.cpp
 * @brief   osu 谱面文件解析与倍速调整核心模块实现
 * @ingroup core
 */

#include "core/FileOperation.hpp"
#include "core/Error.hpp"

#include <cmath>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace core
{
    std::vector<std::string> LoadBeatmapFile(const fs::path &filename)
    {
        if (filename.extension() != ".osu")
        {
            throw MakeParamError("参数不是 osu 谱面文件：" + filename.string());
        }
        std::ifstream ifs(filename);
        if (!ifs)
        {
            throw MakeFileError("无法打开文件", filename.string());
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(ifs, line))
        {
            lines.push_back(line);
        }
        return lines;
    }

    std::vector<std::string>
    ProcessBeatmap(const Difficulty &bmp,
                   std::vector<std::pair<std::string, std::string>> &exinfo,
                   double speed)
    {
        std::vector<std::string> processed{bmp.GetVersion(), ""};

        auto write = [&processed, &bmp](const std::map<std::string, std::string> &map, const std::string &sep = ":") -> void
        {
            for (const auto &key : bmp.GetOrder())
            {
                if (map.count(key))
                {
                    processed.emplace_back(fmt::format("{}{}{}", key, sep, map.at(key)));
                }
            }
            processed.emplace_back("");
        };

        auto map = bmp.GetGeneral();
        if (!map.empty())
        {
            processed.emplace_back("[General]");
            exinfo.emplace_back("af", map["AudioFilename"]);
            if (map.count("PreviewTime") && map["PreviewTime"] != "-1")
            {
                map["PreviewTime"] = fmt::format("{}", std::llround(std::stold(map["PreviewTime"]) / speed));
            }
            write(map, ": ");
        }

        map = bmp.GetEditor();
        if (!map.empty())
        {
            processed.emplace_back("[Editor]");
            write(map, ": ");
        }

        map = bmp.GetMetadata();
        if (!map.empty())
        {
            processed.emplace_back("[Metadata]");
            map["Version"] += fmt::format(" ({:g})", speed);
            map["BeatmapID"] = map["BeatmapSetID"] = "0";
            write(map);
        }

        map = bmp.GetDifficulty();
        if (!map.empty())
        {
            processed.emplace_back("[Difficulty]");
            write(map);
        }

        auto events = bmp.GetEvents();
        if (!events.empty())
        {
            processed.emplace_back("[Events]");
            for (auto &[type, start, args] : events)
            {
                start /= speed;
                if (type == "0")
                {
                    exinfo.emplace_back("bg", args.at(0));
                }
                else if (type == "2" || type == "Break")
                {
                    auto end = args.at(0);
                    args.at(0) = fmt::format("{}", std::llround(std::stoi(end) / speed));
                }
                processed.emplace_back(fmt::format("{},{},{}", type, start, fmt::join(args, ",")));
            }
            processed.emplace_back("");
        }

        auto timepts = bmp.GetTimingPoints();
        if (!timepts.empty())
        {
            processed.emplace_back("[TimingPoints]");
            for (auto &tp : timepts)
            {
                tp.time /= speed;
                if (tp.inherit)
                {
                    tp.beat_length /= speed;
                }
                const auto &[_1, _2, _3, _4, _5, _6, _7, _8] = tp;
                processed.emplace_back(
                    fmt::format("{:g},{:g},{},{},{},{},{:d},{}",
                                _1, _2, _3, _4, _5, _6, _7, _8));
            }
            processed.emplace_back("");
        }

        map = bmp.GetColors();
        if (!map.empty())
        {
            processed.emplace_back("[Colors]");
            write(map, ": ");
        }

        auto objects = bmp.GetObjects();
        if (!objects.empty())
        {
            processed.emplace_back("[HitObjects]");
            for (auto &obj : objects)
            {
                obj.time /= speed;
                char div = ',';
                if (obj.type & (1 << 7) || obj.type & (1 << 3))
                {
                    div = ':';
                    auto end = obj.args.at(0);
                    obj.args.at(0) = fmt::format("{}", std::llround(std::stoi(end) / speed));
                }
                const auto &[_1, _2, _3, _4, _5, _6, _7] = obj;
                auto res = fmt::format("{},{},{},{},{},", _1, _2, _3, _4, _5);
                if (obj.args.empty())
                {
                    res += fmt::format("{}", fmt::join(_7, ":"));
                }
                else
                {
                    res += fmt::format("{}{}{}", fmt::join(_6, "|"), div, fmt::join(_7, ":"));
                }
                processed.emplace_back(res);
            }
        }

        return processed;
    }

    void ExportFile(const fs::path &newdir, const fs::path &name, const std::vector<std::string> &content)
    {
        fs::create_directories(newdir);
        auto outPath = newdir / name.filename();
        std::ofstream ofs(outPath);
        if (!ofs)
        {
            throw MakeFileError("无法创建输出文件", outPath.string());
        }
        for (auto &line : content)
        {
            ofs << line << std::endl;
        }
        if (!ofs)
        {
            throw MakeFileError("写入输出文件失败", outPath.string());
        }
    }
}
