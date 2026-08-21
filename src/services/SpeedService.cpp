/**
 * @file    SpeedService.cpp
 * @brief   倍速服务实现
 * @ingroup services
 */

#include "services/SpeedService.hpp"

#include "common/Utility.hpp"
#include "core/Error.hpp"
#include "core/FileOperation.hpp"
#include "core/beatmap/BeatmapParser.hpp"
#include "core/beatmap/BeatmapSerializer.hpp"
#include "core/speed/SpeedTransform.hpp"

#include <exception>
#include <format>
#include <string>
#include <vector>

#ifdef _WIN32
static const char *kFfprobeBin = "ffprobe.exe";
static const char *kFfmpegBin = "ffmpeg.exe";
#else
static const char *kFfprobeBin = "ffprobe";
static const char *kFfmpegBin = "ffmpeg";
#endif

namespace services
{
    core::fs::path SpeedService::Process(const SpeedRequest &req, ProgressCb onProgress) const
    {
        // 上报进度；返回 false（取消）时抛出内部错误
        auto report = [&](int percent, const std::string &msg)
        {
            if (onProgress && !onProgress(Progress{percent, msg}))
            {
                throw core::MakeInternalError("已取消");
            }
        };

        // 1. 解析谱面
        report(5, "解析谱面");
        auto beatmap = core::BeatmapParser::ParseFile(req.beatmapPath);

        // 2. 倍速变换并抽取音频/背景图信息
        auto [num, denom] = core::speed::ParseFraction(req.tempo.empty() ? req.pitch : req.tempo);
        const auto speed = static_cast<double>(num) / denom;
        core::speed::MediaInfo media;
        auto transformed = core::speed::Transform(beatmap, speed, media);

        if (media.audio.empty() && media.background.empty())
        {
            throw core::MakeFileError("音频或图片信息缺失", req.beatmapPath.string());
        }

        const auto parent = req.beatmapPath.parent_path();
        core::fs::path song, picture;
        if (!media.background.empty())
        {
            picture = parent / common::trim(media.background, " \t\n\r\f\v\"");
        }
        if (!media.audio.empty())
        {
            song = parent / common::trim(media.audio, " \t\n\r\f\v\"");
        }

        // 3. 通过 ffprobe 获取音频采样率
        report(30, "获取音频采样率");
        int sampleRate = 0;
        {
            auto r = runner_.Run(kFfprobeBin,
                                 {"-v", "error", "-select_streams", "a:0", "-show_entries",
                                  "stream=sample_rate", "-of", "default=noprint_wrappers=1:nokey=1",
                                  song.string()});
            if (!r.Succeeded())
            {
                throw core::MakeProcessError("获取音频信息失败：" + r.stdErr, song.string());
            }
            try
            {
                sampleRate = std::stoi(r.stdOut);
            }
            catch (const std::exception &)
            {
                throw core::MakeProcessError("ffprobe 返回的采样率无效：" + r.stdOut, song.string());
            }
        }

        // 4. 导出处理后的谱面
        report(50, "导出谱面文件");
        auto out = req.beatmapPath.stem();
        out.concat(std::format("({:.6g})", static_cast<double>(num) / denom));
        out.concat(req.beatmapPath.extension().string());
        core::ExportFile(req.outputDir, out, core::BeatmapSerializer::ToLines(transformed));

        // 5. 通过 ffmpeg 处理音频
        report(70, "处理音频");
        {
            auto filter = core::speed::GenerateFilterChain(req.tempo, req.pitch, sampleRate, req.mode);
            auto r = runner_.Run(kFfmpegBin,
                                 {"-i", song.string(), "-af", filter, "-loglevel", "error", "-y",
                                  (req.outputDir / song.filename()).string()});
            if (!r.Succeeded())
            {
                throw core::MakeProcessError("音频处理失败：" + r.stdErr, song.string());
            }
        }

        // 6. 复制背景图
        report(90, "复制背景图");
        auto dest = req.outputDir / picture.filename();
        try
        {
            core::fs::copy(picture, dest, core::fs::copy_options::skip_existing);
        }
        catch (const std::exception &e)
        {
            throw core::MakeFileError("复制背景图失败：" + std::string(e.what()), picture.string());
        }

        report(100, "完成");
        return req.outputDir / out;
    }
}
