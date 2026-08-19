/**
 * @file    BeatmapParser.cpp
 * @brief   osu 谱面文件解析器实现
 * @ingroup core
 */

#include "core/beatmap/BeatmapParser.hpp"
#include "core/Error.hpp"
#include "common/Utility.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace core
{
    Beatmap BeatmapParser::Parse(std::istream &in)
    {
        Beatmap result;
        std::string line, section;
        std::map<std::string, std::function<void(const std::string &)>> handlers;

        handlers["[General]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (result.General_.find(key) == result.General_.end())
            {
                result.order_.emplace_back(key);
            }
            result.General_[common::trim(key)] = common::trim(value);
        };
        handlers["[Editor]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (result.Editor_.find(key) == result.Editor_.end())
            {
                result.order_.emplace_back(key);
            }
            result.Editor_[common::trim(key)] = common::trim(value);
        };
        handlers["[Metadata]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (result.Metadata_.find(key) == result.Metadata_.end())
            {
                result.order_.emplace_back(key);
            }
            result.Metadata_[common::trim(key)] = common::trim(value);
        };
        handlers["[Difficulty]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (result.Difficulty_.find(key) == result.Difficulty_.end())
            {
                result.order_.emplace_back(key);
            }
            result.Difficulty_[common::trim(key)] = common::trim(value);
        };
        handlers["[Colors]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (result.Colors_.find(key) == result.Colors_.end())
            {
                result.order_.emplace_back(key);
            }
            result.Colors_[common::trim(key)] = common::trim(value);
        };

        handlers["[Events]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l);
            if (!pl)
            {
                return;
            }
            Beatmap::Event e;
            e.type = pl.NextField();
            e.start_time = std::stoi(pl.NextField());
            while (pl)
            {
                e.args.emplace_back(pl.NextField());
            }
            result.Events_.emplace_back(e);
        };
        handlers["[TimingPoints]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l);
            if (!pl)
            {
                return;
            }
            Beatmap::TimingPoint tp;

            tp.time = std::stod(pl.NextField());
            tp.beat_length = std::stod(pl.NextField());
            tp.meter = std::stod(pl.NextField());
            tp.sound_effect = std::stoi(pl.NextField());
            tp.sound_arg = std::stoi(pl.NextField());
            tp.volume = std::stoi(pl.NextField());
            tp.inherit = std::stoi(pl.NextField());
            tp.effect = std::stoi(pl.NextField());

            result.TimingPoints_.emplace_back(tp);
        };
        handlers["[HitObjects]"] =
            [&result](const std::string &l)
        {
            common::PunctLexer pl(l, ",");
            if (!pl)
            {
                throw 42; // 空行或格式错误，交由外层 catch 存入 Unknown_
            }
            Beatmap::Object o;

            o.x = std::stoi(pl.NextField());
            o.y = std::stoi(pl.NextField());
            o.time = std::stoi(pl.NextField());
            o.type = std::stoi(pl.NextField());
            o.beat_sound = std::stoi(pl.NextField());

            std::string remainder = l.substr(pl.begin() + pl.size() + 1);

            auto colon = remainder.rbegin();
            for (int i = 0; i < 4;)
            {
                if (*colon++ == ':')
                {
                    i++;
                }
            }
            auto div = std::find_if(colon + 1, remainder.rend(), ::ispunct);
            auto args = std::string(remainder.begin(), div.base() - (div == remainder.rend() ? 0 : 1));
            auto sdgrp = std::string(div.base(), remainder.end());

            common::PunctLexer alex(args, "|"), slex(sdgrp, ":");
            while (alex)
            {
                o.args.emplace_back(alex.NextField());
            }
            while (slex)
            {
                o.sound_group.emplace_back(slex.NextField());
            }

            result.Objects_.emplace_back(o);
        };

        while (std::getline(in, line))
        {
            auto trimstr = common::trim(line);
            if (!trimstr.empty() && trimstr.front() == '[' && trimstr.back() == ']')
            {
                section = trimstr;
                continue;
            }
            if (!trimstr.empty() && trimstr.find("osu file format") != std::string::npos)
            {
                result.version = trimstr;
                continue;
            }
            auto func = handlers.find(section);
            if (func != handlers.end())
            {
                try
                {
                    func->second(line);
                }
                catch (...)
                {
                    result.Unknown_.emplace_back(line);
                }
            }
            else
            {
                result.Unknown_.emplace_back(line);
            }
        }

        return result;
    }

    Beatmap BeatmapParser::ParseFile(const fs::path &filename)
    {
        std::ifstream ifs(filename);
        if (!ifs)
        {
            throw MakeFileError("无法打开文件", filename.string());
        }
        return Parse(ifs);
    }
}
