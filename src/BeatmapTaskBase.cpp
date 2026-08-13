/**
 * @file    BeatmapTaskBase.cpp
 * @brief   谱面倍速处理工具同步基类实现
 * @ingroup tools
 */

#include "tools/BeatmapTaskBase.hpp"
#include "core/Error.hpp"
#include "tools/Utility.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
static const char *FFMPEG_BIN = "ffmpeg.exe";
static const char *FFPROBE_BIN = "ffprobe.exe";
#else
#define POPEN popen
#define PCLOSE pclose
static const char *FFMPEG_BIN = "ffmpeg";
static const char *FFPROBE_BIN = "ffprobe";
#endif

namespace tools
{
    void BeatmapTaskBase::Parse(const core::fs::path &newdir)
    {
        // 为新文件加上倍速，方便区分
        auto [num, denom] = core::ParseFraction(tempo_.empty() ? pitch_ : tempo_);
        std::vector<std::pair<std::string, std::string>> exinfo;
        auto lines_ = core::ProcessBeatmap(diff_, exinfo, static_cast<long double>(num) / denom);

        if (exinfo.empty())
        {
            throw core::MakeFileError("音频或图片信息缺失", filename_.string());
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
            throw core::MakeProcessError("获取音频信息失败：" + err, song_.string());
        }

        auto out = filename_.stem();
        out.concat(fmt::format("({:.6g})", static_cast<long double>(num) / denom));
        out.concat(filename_.extension().string());

        core::ExportFile(newdir, out, lines_);

        if (!ProcessAudio(newdir, &err))
        {
            throw core::MakeProcessError("音频处理失败：" + err, song_.string());
        }
        auto dest = newdir / picture_.filename();
        try
        {
            core::fs::copy(picture_, dest, core::fs::copy_options::skip_existing);
        }
        catch (const std::exception &e)
        {
            throw core::MakeFileError("复制背景图失败：" + std::string(e.what()), picture_.string());
        }
    }

    bool BeatmapTaskBase::FetchAudioInfo(std::string *errorMsg)
    {
        auto command = fmt::format(
            "{} -v error -select_streams a:0 -show_entries stream=sample_rate -of default=noprint_wrappers=1:nokey=1 \"{}\"",
            FFPROBE_BIN, song_.string());
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

    bool BeatmapTaskBase::ProcessAudio(const core::fs::path &newdir, std::string *errorMsg) const
    {
        try
        {
            // 构造滤镜链
            auto command = core::GenerateFilterChain(tempo_, pitch_, sampleRate_, opt_);

            command = fmt::format("{} -i \"{}\" {} -loglevel error -y \"{}\"", FFMPEG_BIN, song_.string(), command, (newdir / song_.filename()).string());

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
