/**
 * @file    SpeedTransform.cpp
 * @brief   谱面倍速变换纯算法实现
 * @ingroup core
 */

#include "core/speed/SpeedTransform.hpp"

#include <cmath>
#include <string>

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>

namespace core::speed
{
    Beatmap Transform(const Beatmap &src, double speed, MediaInfo &media)
    {
        Beatmap result = src;

        // [General]：缩放 PreviewTime，抽取音频文件名
        auto general = result.GetGeneral();
        if (!general.empty())
        {
            media.audio = general["AudioFilename"];
            if (general.count("PreviewTime") && general["PreviewTime"] != "-1")
            {
                general["PreviewTime"] = fmt::format("{}", std::llround(std::stold(general["PreviewTime"]) / speed));
            }
        }
        result.SetGeneral(std::move(general));

        // [Metadata]：版本追加倍速标记，谱面 ID 置 0
        auto metadata = result.GetMetadata();
        if (!metadata.empty())
        {
            metadata["Version"] += fmt::format(" ({:g})", speed);
            metadata["BeatmapID"] = metadata["BeatmapSetID"] = "0";
        }
        result.SetMetadata(std::move(metadata));

        // [Events]：缩放开始/结束时间，抽取背景图文件名
        auto events = result.GetEvents();
        for (auto &e : events)
        {
            e.start_time = static_cast<int>(e.start_time / speed);
            if (e.type == "0")
            {
                media.background = e.args.at(0);
            }
            else if (e.type == "2" || e.type == "Break")
            {
                e.args.at(0) = fmt::format("{}", std::llround(std::stoi(e.args.at(0)) / speed));
            }
        }
        result.SetEvents(std::move(events));

        // [TimingPoints]：缩放时间与继承点的节拍长度
        auto timepts = result.GetTimingPoints();
        for (auto &tp : timepts)
        {
            tp.time /= speed;
            if (tp.inherit)
            {
                tp.beat_length /= speed;
            }
        }
        result.SetTimingPoints(std::move(timepts));

        // [HitObjects]：缩放出现时间与滑条/转盘结束时间
        auto objects = result.GetObjects();
        for (auto &obj : objects)
        {
            obj.time = static_cast<int>(obj.time / speed);
            if (obj.type & (1 << 7) || obj.type & (1 << 3))
            {
                obj.args.at(0) = fmt::format("{}", std::llround(std::stoi(obj.args.at(0)) / speed));
            }
        }
        result.SetObjects(std::move(objects));

        return result;
    }
}
