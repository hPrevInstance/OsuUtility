/**
 * @file    Difficulty.cpp
 * @brief   osu 谱面文件解析类实现
 * @ingroup core
 */

#include "core/Difficulty.hpp"
#include "core/Error.hpp"
#include "tools/Utility.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace core
{
    void Difficulty::load()
    {
        order_.clear();
        std::ifstream ifs(filename_);
        if (!ifs)
        {
            throw MakeFileError("无法打开文件", filename_.string());
        }

        std::string line, section;
        std::map<std::string, std::function<void(const std::string &)>> handlers;

        handlers["[General]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (General_.find(key) == General_.end())
            {
                order_.emplace_back(key);
            }
            General_[tools::trim(key)] = tools::trim(value);
        };
        handlers["[Editor]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (Editor_.find(key) == Editor_.end())
            {
                order_.emplace_back(key);
            }
            Editor_[tools::trim(key)] = tools::trim(value);
        };
        handlers["[Metadata]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (Metadata_.find(key) == Metadata_.end())
            {
                order_.emplace_back(key);
            }
            Metadata_[tools::trim(key)] = tools::trim(value);
        };
        handlers["[Difficulty]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (Difficulty_.find(key) == Difficulty_.end())
            {
                order_.emplace_back(key);
            }
            Difficulty_[tools::trim(key)] = tools::trim(value);
        };
        handlers["[Colors]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l, ":");
            if (!pl)
            {
                return;
            }
            std::string key = pl.NextField();
            std::string value = pl.NextField();
            if (Colors_.find(key) == Colors_.end())
            {
                order_.emplace_back(key);
            }
            Colors_[tools::trim(key)] = tools::trim(value);
        };

        handlers["[Events]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l);
            if (!pl)
            {
                return;
            }
            Event e;
            e.type = pl.NextField();
            e.start_time = std::stoi(pl.NextField());
            while (pl)
            {
                e.args.emplace_back(pl.NextField());
            }
            Events_.emplace_back(e);
        };
        handlers["[TimingPoints]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l);
            if (!pl)
            {
                return;
            }
            TimingPoint tp;

            tp.time = std::stod(pl.NextField());
            tp.beat_length = std::stod(pl.NextField());
            tp.meter = std::stod(pl.NextField());
            tp.sound_effect = std::stoi(pl.NextField());
            tp.sound_arg = std::stoi(pl.NextField());
            tp.volume = std::stoi(pl.NextField());
            tp.inherit = std::stoi(pl.NextField());
            tp.effect = std::stoi(pl.NextField());

            TimingPoints_.emplace_back(tp);
        };
        handlers["[HitObjects]"] =
            [this](const std::string &l)
        {
            tools::PunctLexer pl(l, ",");
            if (!pl)
            {
                throw 42;
            }
            Object o;

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

            tools::PunctLexer alex(args, "|"), slex(sdgrp, ":");
            while (alex)
            {
                o.args.emplace_back(alex.NextField());
            }
            while (slex)
            {
                o.sound_group.emplace_back(slex.NextField());
            }

            Objects_.emplace_back(o);
        };

        while (std::getline(ifs, line))
        {
            auto trimstr = tools::trim(line);
            if (!trimstr.empty() && trimstr.front() == '[' && trimstr.back() == ']')
            {
                section = trimstr;
                continue;
            }
            if (!trimstr.empty() && trimstr.find("osu file format") != std::string::npos)
            {
                version = trimstr;
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
                    Unknown_.emplace_back(line);
                }
            }
            else
            {
                Unknown_.emplace_back(line);
            }
        }
    }
}
