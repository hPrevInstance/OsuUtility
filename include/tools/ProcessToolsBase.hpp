#pragma once

#include "core/AudioProcess.hpp"
#include "core/FileOperation.hpp"
#include <functional>

namespace tools
{
    class BeatmapTaskBase
    {
    protected:
        core::fs::path beatmap_, song_;
        std::string tempo_, pitch_;
        int sampleRate_, duration_;
        inline static core::Option opt_;
        std::vector<std::string> lines;

    public:
        BeatmapTaskBase(const core::fs::path &bmp) : beatmap_(bmp)
        {
            lines = core::LoadBeatmapFile(beatmap_);
            auto it = std::find_if(lines.begin(), lines.end(), [](const std::string &str)
                                   { return str.find("AudioFilename") != std::string::npos; });
            if (it != lines.end())
            {
                auto pos = it->find(':');
                core::fs::path song(it->substr(pos + 1));
                song_ = beatmap_.parent_path() / song;
            }
            else
            {
                throw std::invalid_argument("格式错误：该谱面中找不到歌曲名：" + beatmap_.filename().string());
            }
        }
        static void SetGlobalOption(core::Option opt)
        {
            opt_ = opt;
        }
        void SetPitch(const std::string &p)
        {
            pitch_ = p;
        }
        void SetTempo(const std::string &t)
        {
            tempo_ = t;
        }
        std::vector<std::string> ParseBeatmap();
    };
}