/**
 * @file    FileOperation.hpp
 * @brief   osu谱面文件解析与倍速调整核心模块
 *
 * 该模块提供了完整的 osu谱面文件读写和倍速缩放功能。
 *
 * @author  hPrevInstance
 * @date    2026-08-11
 * @version 1.0.0
 * @ingroup core
 *
 * @see     https://osu.ppy.sh/wiki/zh/Client/File_formats/osu_(file_format)
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include "Difficulty.hpp"
#include "tools/Utility.hpp"

/**
 * @defgroup core 核心算法模块
 * @brief 包含所有与界面无关的文件处理与谱面解析逻辑
 */
namespace core
{
    namespace fs = std::filesystem;

    /**
     * @brief 加载指定的osu谱面文件
     *
     * @param filename 该文件的路径
     * @return std::vector<std::string> 分割为字符串向量的osu文件
     * @throw std::invaild_argument 当不是osu文件或打开失败时抛出
     */
    inline std::vector<std::string> LoadBeatmapFile(const fs::path &filename)
    {
        if (filename.extension() != ".osu")
        {
            throw std::invalid_argument("参数错误：参数不是osu谱面文件：" + filename.string());
        }
        std::ifstream ifs(filename);
        if (!ifs)
        {
            throw std::runtime_error("系统错误：无法打开文件：" + filename.string());
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(ifs, line))
        {
            lines.push_back(line);
        }
        return lines;
    }
    /**
     * @brief 处理osu谱面文件的各个章节的时间倍速
     *
     * @param bmp 谱面对象
     * @param speed 倍速
     * @param exinfo 处理过程的额外信息
     * @return std::vector<std::string> 返回处理后的文件向量
     * @throw 当文件头不是官方指定或空文件时抛出
     */
    inline std::vector<std::string>
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
    /**
     * @brief 将处理好的文件导出到newdir，如果不存在则自动创建
     *
     * @param newdir 新目录名称
     * @param name 文件名
     * @param content 已处理的文件内容
     */
    inline void ExportFile(const fs::path &newdir, const fs::path &name, const std::vector<std::string> &content)
    {
        fs::create_directories(newdir);
        std::ofstream ofs(newdir / name.filename());
        for (auto &line : content)
        {
            ofs << line << std::endl;
        }
    }
}