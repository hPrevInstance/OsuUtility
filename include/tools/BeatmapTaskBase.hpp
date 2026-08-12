/**
 * @file    BeatmapTaskBase.hpp
 * @brief   谱面倍速处理工具的同步基类
 *
 * 该模块提供了谱面处理流程的串行实现：读取谱面信息、调用 ffprobe 获取
 * 音频采样率、处理谱面文件文本、调用 ffmpeg 处理音频并导出结果。
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup tools
 */

#pragma once

#include "core/AudioProcess.hpp"
#include "core/FileOperation.hpp"
#include "tools/Utility.hpp"
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
     * 音频文件的元信息
     */
    class SongInfo
    {
    };

    /**
     * @brief 谱面处理基类，用于CLI同步处理
     *
     * 封装了一次完整的谱面倍速处理流程所需的全部状态与方法：
     *  - 谱面、音频、背景图的路径
     *  - 变速/变调参数与模式
     *  - 音频信息获取与音频重采样处理
     */
    class BeatmapTaskBase
    {
    protected:
        core::fs::path filename_, song_, picture_; // 谱面、歌曲路径、背景图
        std::string tempo_, pitch_;                // 变速变调
        core::Option opt_ = 0;                     // 选项
        core::Difficulty diff_;                    // 已解析的谱面对象

        /**
         * @brief 获取音频的采样率信息
         *
         * 通过调用 ffprobe 读取音频流采样率并保存到 sampleRate_。
         * 派生类可覆写以实现不同的实现方式。
         *
         * @param errorMsg 失败时输出的错误信息
         * @return true 成功
         * @return false 失败
         */
        virtual bool FetchAudioInfo(std::string *errorMsg);

        /**
         * @brief 处理谱面音频
         *
         * @param newdir   输出目录
         * @param errorMsg 失败时输出的错误信息
         * @return true 成功
         * @return false 失败
         */
        virtual bool ProcessAudio(const core::fs::path &newdir, std::string *errorMsg) const;

    private:
        int sampleRate_; // 音频采样率

    public:
        /**
         * @brief 处理器构造函数
         *
         * @param bmp 谱面的路径
         */
        BeatmapTaskBase(const core::fs::path &p) : diff_(p)
        {
            filename_ = p;
            diff_.load();
        }
        BeatmapTaskBase(const BeatmapTaskBase &) = default;
        BeatmapTaskBase(BeatmapTaskBase &&) = default;

        /// @brief 设置处理模式
        void SetMode(core::Option opt)
        {
            opt_ = opt;
        }
        /// @brief 设置变调参数
        void SetPitch(const std::string &p)
        {
            pitch_ = p;
        }
        /// @brief 设置变速参数
        void SetTempo(const std::string &t)
        {
            tempo_ = t;
        }
        /**
         * @brief 执行完整的处理流程并输出到新目录
         *
         * @param newdir 输出到的目录
         */
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
        // 为新文件加上倍速，方便区分
        auto [num, denom] = core::ParseFraction(tempo_.empty() ? pitch_ : tempo_);
        std::vector<std::pair<std::string, std::string>> exinfo;
        auto lines_ = core::ProcessBeatmap(diff_, exinfo, static_cast<long double>(num) / denom);

        if (exinfo.empty())
        {
            throw std::invalid_argument("错误：音频或图片信息缺失 文件：" + filename_.string());
        }

        // 获取音乐和背景信息
        for (const auto &[type, info] : exinfo)
        {
            if (type == "bg")
            {
                picture_ = info;
                picture_ = filename_.parent_path() / trim(picture_.string(), " \t\n\r\f\v\"");
            }
            else if (type == "af")
            {
                song_ = info;
                song_ = filename_.parent_path() / trim(song_.string(), " \t\n\r\f\v\"");
            }
        }

        std::string err;
        if (!FetchAudioInfo(&err))
        {
            throw std::runtime_error("错误：获取音频信息失败：" + err);
        }

        auto out = filename_.stem();
        out.concat(fmt::format("({:.6g})", static_cast<long double>(num) / denom));
        out.concat(filename_.extension().string());

        core::ExportFile(newdir, out, lines_);

        if (!ProcessAudio(newdir, &err))
        {
            throw std::runtime_error("错误：音频处理失败：" + err);
        }
        auto dest = newdir / picture_.filename();
        core::fs::copy(picture_, dest, core::fs::copy_options::skip_existing);
    }

    /**
     * @brief 通过 ffprobe 获取音频采样率
     *
     * 构造 ffprobe 命令读取第一条音频流的采样率，解析输出并保存到 sampleRate_。
     *
     * @param errorMsg 失败时输出的错误信息
     * @return true 成功
     * @return false 失败
     */
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
            ffprobe, song_.string());
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
                *errorMsg = fmt::format("ffprobe返回的采样率无效：{} 当前文件：{}", res, song_.string());
            }
            return false;
        }

        return true;
    }

    /**
     * @brief 调用 ffmpeg 处理音频
     *
     * 基于变速/变调参数构造滤镜链，使用 ffmpeg 对音频进行重采样并输出到新目录。
     *
     * @param newdir   新路径
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

            command = fmt::format("{} -i \"{}\" {} -loglevel error -y \"{}\"", ffmpeg, song_.string(), command, (newdir / song_.filename()).string());

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
}