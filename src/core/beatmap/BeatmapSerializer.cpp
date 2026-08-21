/**
 * @file    BeatmapSerializer.cpp
 * @brief   osu 谱面序列化器实现
 * @ingroup core
 */

#include "core/beatmap/BeatmapSerializer.hpp"

#include "common/Utility.hpp"

#include <format>
#include <map>
#include <string>
#include <vector>

namespace core
{
    std::vector<std::string> BeatmapSerializer::ToLines(const Beatmap &bmp)
    {
        std::vector<std::string> processed{bmp.GetVersion(), ""};

        auto write = [&processed, &bmp](const std::map<std::string, std::string> &map, const std::string &sep = ":") -> void
        {
            for (const auto &key : bmp.GetOrder())
            {
                if (map.count(key))
                {
                    processed.emplace_back(std::format("{}{}{}", key, sep, map.at(key)));
                }
            }
            processed.emplace_back("");
        };

        const auto &general = bmp.GetGeneral();
        if (!general.empty())
        {
            processed.emplace_back("[General]");
            write(general, ": ");
        }

        const auto &editor = bmp.GetEditor();
        if (!editor.empty())
        {
            processed.emplace_back("[Editor]");
            write(editor, ": ");
        }

        const auto &metadata = bmp.GetMetadata();
        if (!metadata.empty())
        {
            processed.emplace_back("[Metadata]");
            write(metadata);
        }

        const auto &difficulty = bmp.GetDifficulty();
        if (!difficulty.empty())
        {
            processed.emplace_back("[Difficulty]");
            write(difficulty);
        }

        const auto &events = bmp.GetEvents();
        if (!events.empty())
        {
            processed.emplace_back("[Events]");
            for (const auto &e : events)
            {
                processed.emplace_back(std::format("{},{},{}", e.type, e.start_time, common::join(e.args, ",")));
            }
            processed.emplace_back("");
        }

        const auto &timepts = bmp.GetTimingPoints();
        if (!timepts.empty())
        {
            processed.emplace_back("[TimingPoints]");
            for (const auto &tp : timepts)
            {
                processed.emplace_back(
                    std::format("{:g},{:g},{},{},{},{},{:d},{}",
                                tp.time, tp.beat_length, tp.meter, tp.sound_effect, tp.sound_arg, tp.volume,
                                static_cast<int>(tp.inherit), tp.effect));
            }
            processed.emplace_back("");
        }

        const auto &colors = bmp.GetColors();
        if (!colors.empty())
        {
            processed.emplace_back("[Colors]");
            write(colors, ": ");
        }

        const auto &objects = bmp.GetObjects();
        if (!objects.empty())
        {
            processed.emplace_back("[HitObjects]");
            for (const auto &obj : objects)
            {
                char div = ',';
                if (obj.type & (1 << 7) || obj.type & (1 << 3))
                {
                    div = ':';
                }
                auto res = std::format("{},{},{},{},{},", obj.x, obj.y, obj.time, obj.type, obj.beat_sound);
                if (obj.args.empty())
                {
                    res += std::format("{}", common::join(obj.sound_group, ":"));
                }
                else
                {
                    res += std::format("{}{}{}", common::join(obj.args, "|"), div, common::join(obj.sound_group, ":"));
                }
                processed.emplace_back(res);
            }
        }

        return processed;
    }
}
