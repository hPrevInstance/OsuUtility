#pragma once

#include "core/AudioProcess.hpp"
#include "core/FileOperation.hpp"
#include <functional>
#include <cstdio>

/**
 * @brief 谱面处理工具
 *
 * @defgroup tools
 */
namespace tools
{

    /**
     * @brief  存放歌曲元数据
     *
     */
    class SongInfo
    {
    };

    /**
     * @brief 谱面处理基类，用于CLI同步处理
     *
     */
    class BeatmapTaskBase
    {
    protected:
        core::fs::path beatmap_, song_;  // 谱面和歌曲路径
        std::string tempo_, pitch_;      // 变速变调
        inline static core::Option opt_; // 选项
        std::vector<std::string> lines_;

        virtual bool FetchAudioInfo(std::string *errorMsg = nullptr);
        virtual bool ProcessAudio(const core::fs::path &newdir, std::string *errorMsg = nullptr) const;

    private:
        int sampleRate_;

    public:
        /**
         * @brief 处理器构造函数
         *
         * @param bmp 谱面的路径
         */
        BeatmapTaskBase(const core::fs::path &bmp) : beatmap_(bmp)
        {
            lines_ = core::LoadBeatmapFile(beatmap_);

            // 寻找音频文件
            auto it = std::find_if(lines_.begin(), lines_.end(), [](const std::string &str)
                                   { return str.find("AudioFilename") != std::string::npos; });
            if (it != lines_.end())
            {
                auto pos = it->find(':');
                core::fs::path song(it->substr(pos + 1));
                song_ = beatmap_.parent_path() / song;
            }
            else
            {
                throw std::invalid_argument("格式错误：该谱面中找不到歌曲文件：" + beatmap_.filename().string());
            }
        }
        BeatmapTaskBase(const BeatmapTaskBase &) = default;
        BeatmapTaskBase(BeatmapTaskBase &&) = default;

        static void SetMode(core::Option opt)
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
        virtual void Parse(const core::fs::path &newdir);
        virtual ~BeatmapTaskBase() = default;
    };

    /**
     * @brief 谱面处理接口
     *
     * @param newdir 输出到的目录
     */
    void BeatmapTaskBase::Parse(const core::fs::path &newdir)
    {
        std::string err;
        if (!FetchAudioInfo(&err))
        {
            throw std::runtime_error("错误：获取音频信息失败：" + err);
        }

        // 为新文件加上倍速，方便区分
        auto [num, denom] = core::ParseFraction(tempo_);
        lines_ = core::ProcessBeatmap(beatmap_, lines_, static_cast<long double>(num) / denom);

        auto out = beatmap_.stem();
        out.concat(fmt::format("({:.6g})", static_cast<long double>(num) / denom));
        out.concat(beatmap_.extension().string());

        core::ExportFile(newdir, out, lines_);

        if (!ProcessAudio(newdir, &err))
        {
            throw std::runtime_error("错误：音频处理失败：" + err);
        }
    }

    bool BeatmapTaskBase::FetchAudioInfo(std::string *errorMsg = nullptr)
    {

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
        const char *ffprobe = "ffprobe.exe";
#else
#define POPEN popen
#define PCLOSE pclose
        const char *ffprobe = "ffprobe";
#endif

        auto command = fmt::format(
            "{} -v error -select_streams a:0 -show_entries stream=sample_rate -of default=noprint_wrappers=1:nokey=1 \"{}\"",
            ffprobe, song_);
        char buffer[BUFSIZ]{};
        std::string res;

        // 抓取采样率输出
        FILE *pipe = POPEN(command.c_str(), "r");
        if (!pipe)
        {
            if (errorMsg != nullptr)
            {
                *errorMsg = "ffprobe管道打开失败 当前文件：" + song_.string();
            }
            return false;
        }
        while (fgets(buffer, BUFSIZ - 1, pipe) != nullptr)
        {
            res += buffer;
        }
        int status = PCLOSE(pipe);
        if (status == -1)
        {
            if (errorMsg != nullptr)
            {
                *errorMsg = "ffprobe管道关闭失败 当前文件：" + song_.string();
            }
            return false;
        }
        else if (status != 0)
        {
            if (errorMsg != nullptr)
            {
                *errorMsg = "ffprobe命令行解析错误 当前文件：" + song_.string();
            }
            return false;
        }

        try
        {
            sampleRate_ = std::stoi(res);
        }
        catch (const std::exception &e)
        {
            if (errorMsg != nullptr)
            {
                *errorMsg = fmt::format("ffprobe返回的采样率无效：{} 当前文件：{}", res, song_);
            }
            return false;
        }

        return true;
    }

    /**
     * @brief 处理谱面的音频
     *
     * @param newdir 新路径
     * @param errorMsg 错误信息
     * @return true 成功
     * @return false 失败
     */
    bool BeatmapTaskBase::ProcessAudio(const core::fs::path &newdir, std::string *errorMsg = nullptr) const
    {

#ifdef _WIN32
        const char *ffmpeg = "ffmpeg.exe";
#else
        const char *ffmpeg = "ffmpeg";
#endif

        try
        {
            // 构造滤镜链
            auto command = core::GenerateFilterChain(tempo_, pitch_, sampleRate_, opt_);

            auto [num, denom] = core::ParseFraction(tempo_);
            auto out = song_.stem();
            out.concat(fmt::format("({:.6g})", static_cast<long double>(num) / denom));
            out.concat(song_.extension().string());

            command = fmt::format("{} -i \"{}\" {} -y \"{}\"", ffmpeg, song_, command, newdir / out);

            int ret = std::system(command.c_str());
            if (ret != 0)
            {
                if (errorMsg)
                {
                    *errorMsg = fmt::format("ffmpeg 执行失败，退出码: {}", ret);
                }
                return false;
            }
        }
        catch (const std::exception &e)
        {
            if (errorMsg != nullptr)
            {
                *errorMsg = e.what();
                return false;
            }
        }
        return true;
    }

    std::string trim(const std::string &s)
    {
        auto start = s.find_first_not_of(" \t\n\r\f\v");
        auto end = s.find_last_not_of(" \t\n\r\f\v");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
}