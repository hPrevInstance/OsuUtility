/**
 * @file    SpeedCommand.cpp
 * @brief   speed 子命令实现
 * @ingroup cli
 */

#include "cli/SpeedCommand.hpp"
#include "cli/CLIHandler.hpp"
#include "core/Error.hpp"
#include "adapters/QProcessRunner.hpp"
#include "services/SpeedService.hpp"
#include "common/Utility.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>

namespace cli
{
    //  路径工具
    namespace
    {
        std::string Lower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return s;
        }

        bool IsInside(const core::fs::path &path, const core::fs::path &dir)
        {
            auto p = core::fs::absolute(path).lexically_normal().string();
            auto d = core::fs::absolute(dir).lexically_normal().string();
#ifdef _WIN32
            p = Lower(p);
            d = Lower(d);
#endif
            if (p == d)
            {
                return true;
            }
            if (p.size() <= d.size() || p.compare(0, d.size(), d) != 0)
            {
                return false;
            }
            char sep = p[d.size()];
            return sep == '/' || sep == '\\';
        }
    } // namespace

    //  速度解析与校验
    std::string ValidateSpeed(const std::string &input)
    {
        auto value = common::trim(input);
        if (value.empty())
        {
            return "倍速不应为空";
        }
        if (value.find('-') != std::string::npos)
        {
            return "倍速必须为正数：" + value;
        }
        try
        {
            auto [num, denom] = core::speed::ParseFraction(value);
            if (num <= 0 || denom <= 0)
            {
                return "倍速必须为正数：" + value;
            }
        }
        catch (const std::exception &e)
        {
            return "无法解析为倍速：" + value;
        }
        return std::string();
    }

    //  映射文件
    namespace
    {
        std::vector<std::string> SplitFields(const std::string &line)
        {
            std::vector<std::string> fields;
            std::string cur;
            bool inQuote = false;
            for (char c : line)
            {
                if (c == '\"')
                {
                    inQuote = !inQuote;
                }
                else if ((c == ' ' || c == '\t') && !inQuote)
                {
                    if (!cur.empty())
                    {
                        fields.push_back(cur);
                        cur.clear();
                    }
                }
                else
                {
                    cur.push_back(c);
                }
            }
            if (!cur.empty())
            {
                fields.push_back(cur);
            }
            return fields;
        }

        std::map<std::string, DiffEntry> LoadDiffMapping(const core::fs::path &file)
        {
            std::map<std::string, DiffEntry> mapping;
            std::ifstream ifs(file);
            if (!ifs)
            {
                throw core::MakeFileError("无法打开映射文件", file.string());
            }

            std::string line;
            std::size_t lineno = 0;
            while (std::getline(ifs, line))
            {
                ++lineno;
                auto trimmed = common::trim(line);
                if (trimmed.empty() || trimmed.front() == '#')
                {
                    continue;
                }

                auto fields = SplitFields(trimmed);
                if (fields.size() < 2 || fields.size() > 3)
                {
                    throw core::MakeParamError(
                        fmt::format("文件{}第 {} 行格式错误，应为：文件名 变速 [变调]：{}", lineno, line, file.string()));
                }

                DiffEntry entry;
                entry.tempo = fields[1];
                entry.pitch = fields.size() >= 3 ? fields[2] : std::string();

                try
                {
                    if (!entry.tempo.empty())
                    {
                        core::speed::ParseFraction(entry.tempo);
                    }
                    if (!entry.pitch.empty())
                    {
                        core::speed::ParseFraction(entry.pitch);
                    }
                }
                catch (const std::exception &e)
                {
                    throw core::MakeParamError(fmt::format("文件{}第 {} 行：{}", lineno, e.what(), file.string()));
                }

                if (entry.tempo.empty() && entry.pitch.empty())
                {
                    throw core::MakeParamError(fmt::format("文件{}第 {} 行：变速与变调均为空", lineno, file.string()));
                }

                if (!mapping.emplace(fields[0], std::move(entry)).second)
                {
                    throw core::MakeParamError(fmt::format("文件{}第 {} 行：谱面「{}」重复定义", lineno, fields[0], file.string()));
                }
            }
            return mapping;
        }

        //  输入路径收集
        std::vector<core::fs::path> CollectBeatmapPaths(const std::vector<core::fs::path> &files,
                                                        const core::fs::path &output,
                                                        bool recursive)
        {
            std::set<core::fs::path> paths;

            auto insert = [&](const core::fs::path &p)
            {
                if (core::fs::is_regular_file(p) && p.extension() == ".osu" && !IsInside(p, output))
                {
                    paths.insert(p);
                }
            };

            for (const auto &path : files)
            {
                if (core::fs::is_regular_file(path))
                {
                    insert(path);
                }
                else if (core::fs::is_directory(path))
                {
                    if (IsInside(path, output))
                    {
                        continue;
                    }
                    try
                    {
                        if (recursive)
                        {
                            for (const auto &entry : core::fs::recursive_directory_iterator(path))
                            {
                                insert(entry.path());
                            }
                        }
                        else
                        {
                            for (const auto &entry : core::fs::directory_iterator(path))
                            {
                                insert(entry.path());
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        throw core::MakeFileError("遍历目录失败：" + std::string(e.what()), path.string());
                    }
                }
            }
            return std::vector<core::fs::path>(paths.begin(), paths.end());
        }

        //  外部工具检测
        bool CommandExists(const std::string &cmd)
        {
#ifdef _WIN32
            std::string check = "where " + cmd + " >nul 2>nul";
#else
            std::string check = "command -v " + cmd + " >/dev/null 2>&1";
#endif
            return std::system(check.c_str()) == 0;
        }

        std::string FFmpegName()
        {
#ifdef _WIN32
            return "ffmpeg.exe";
#else
            return "ffmpeg";
#endif
        }

        std::string FFprobeName()
        {
#ifdef _WIN32
            return "ffprobe.exe";
#else
            return "ffprobe";
#endif
        }

        //  交互输入
        std::string PromptSpeed(const std::string &label)
        {
            while (true)
            {
                std::cout << "  请输入" << label << std::flush;
                std::string value;
                std::getline(std::cin, value);
                value = common::trim(value);
                if (value.empty())
                {
                    return value;
                }
                auto err = ValidateSpeed(value);
                if (err.empty())
                {
                    return value;
                }
                std::cerr << Red("  [错误] ") << err << std::endl;
            }
        }
    } // namespace

    //  SpeedCommand 实现
    void SpeedCommand::Register(CLI::App &app)
    {
        sub_ = app.add_subcommand("speed", "批量调整 .osu 谱面与音频的变速/变调");

        sub_->add_option("-i,--input", files_, "要转换的文件或目录，可指定多个；目录会被遍历")
            ->required()
            ->check(CLI::ExistingPath)
            ->type_name("PATH...");

        sub_->add_option("-o,--output", output_, "输出目录，处理结果写入其中")
            ->required()
            ->type_name("DIR");

        sub_->add_flag("-r,--recursive", recursive_, "递归遍历输入目录下的所有子目录");

        //  速度选项
        auto *speedGroup = sub_->add_option_group("速度设置", "为所有谱面设置同一速度，支持小数或分数");
        auto *optSpeed = speedGroup->add_option("-s,--speed", speed_, "同时设置变速与变调");
        auto *optTempo = speedGroup->add_option("-t,--tempo", tempo_, "设置变速");
        auto *optPitch = speedGroup->add_option("-p,--pitch", pitch_, "设置变调");
        optSpeed->check(SpeedValidator())->type_name("SPEED");
        optTempo->check(SpeedValidator())->type_name("SPEED");
        optPitch->check(SpeedValidator())->type_name("SPEED");
        optSpeed->excludes(optTempo, optPitch);

        //  差异模式
        optDiff_ = sub_->add_option("-d,--diff", diffFile_, "为每个谱面单独设置速度：可指定映射文件->每行：文件名 变速 [变调]，省略则交互式逐文件询问")
                       ->expected(0, 1)
                       ->check(CLI::ExistingFile)
                       ->type_name("[FILE]");
        optDiff_->excludes(optSpeed, optTempo, optPitch);

        //  行为选项
        sub_->add_flag("-n,--dry-run", dryRun_, "仅列出将执行的操作，不真正处理");
        sub_->add_flag("-q,--quiet", quiet_, "只输出错误与最终统计，抑制逐文件进度");
        sub_->add_flag("--verbose", verbose_, "输出更详细的调试信息");
        sub_->add_flag("--color", forceColor_, "强制启用彩色输出");

        sub_->footer("示例：\n"
                     "  outil speed -i song.osu -o out -s 1.5\n"
                     "  outil speed -i songs/ -r -o out -t 3/2\n"
                     "  outil speed -i song.osu -o out -d\n"
                     "  outil speed -i song.osu -o out -d speed.map\n"
                     "  outil speed -i song.osu -o out -s 2 -n --color");
    }

    int SpeedCommand::Run()
    {
        g_useColor = forceColor_ || DetectColor();

        bool diffMode = optDiff_ != nullptr && optDiff_->count() > 0;

        //  非差异模式下至少需要一个速度选项
        if (!diffMode && speed_.empty() && tempo_.empty() && pitch_.empty())
        {
            std::cerr << Red("[错误] ") << "请指定速度：--speed、--tempo 或 --pitch 至少提供一个，或使用 --diff 逐谱面设置" << std::endl;
            return core::EXIT_USAGE;
        }

        //  解析差异模式映射文件
        std::optional<std::map<std::string, DiffEntry>> mapping;
        if (diffMode && !diffFile_.empty())
        {
            mapping = LoadDiffMapping(diffFile_);
            if (mapping->empty())
            {
                std::cerr << Yellow("[警告] ") << "映射文件为空：" << diffFile_ << std::endl;
            }
        }

        //  解析全局速度
        std::string globalTempo = tempo_;
        std::string globalPitch = pitch_;
        if (!speed_.empty())
        {
            globalTempo = globalPitch = speed_;
        }

        //  收集待处理的谱面
        auto paths = CollectBeatmapPaths(files_, output_, recursive_);
        if (paths.empty())
        {
            std::cerr << Yellow("[警告] ") << "未找到任何 .osu 谱面文件。" << std::endl;
            return core::EXIT_USAGE;
        }

        //  外部工具检测
        if (!dryRun_ && (!CommandExists(FFmpegName()) || !CommandExists(FFprobeName())))
        {
            std::cerr << Red("[错误] ") << "未检测到 ffmpeg / ffprobe，请将其加入 PATH 后再运行。" << std::endl;
            return core::EXIT_EXTERNAL;
        }

        if (!quiet_)
        {
            std::cout << Cyan("== osu! 谱面倍速调节器 ==") << std::endl;
            std::cout << "待处理谱面数：" << paths.size() << std::endl;
            if (dryRun_)
            {
                std::cout << Yellow("（试运行模式：不会真正修改任何文件）") << std::endl;
            }
            if (verbose_)
            {
                std::cout << Dim("输出目录：" + core::fs::absolute(output_).string()) << std::endl;
            }
            std::cout << std::endl;
        }

        //  逐文件处理
        std::size_t success = 0, skipped = 0, failed = 0;
        std::vector<std::pair<std::string, std::string>> failures;

        adapters::QProcessRunner runner;
        services::SpeedService service(runner);

        std::size_t index = 0;
        for (const auto &path : paths)
        {
            ++index;
            try
            {
                std::string dp = globalPitch, dt = globalTempo;

                if (diffMode)
                {
                    if (mapping.has_value())
                    {
                        auto it = mapping->find(path.filename().string());
                        if (it == mapping->end())
                        {
                            if (!quiet_)
                            {
                                std::cout << Yellow(fmt::format("[{}/{}] 跳过，映射文件中未定义：{}", index, paths.size(), path.filename().string())) << std::endl;
                            }
                            ++skipped;
                            continue;
                        }
                        dp = it->second.pitch;
                        dt = it->second.tempo;
                    }
                    else
                    {
                        std::cout << Cyan(fmt::format("[{}/{}] 谱面：{}", index, paths.size(), path.filename().string())) << std::endl;
                        dt = PromptSpeed("变速");
                        dp = PromptSpeed("变调");
                        if (dp.empty() && dt.empty())
                        {
                            dp = dt = "1";
                        }
                    }
                }

                if (dp.empty() && dt.empty())
                {
                    dp = dt = "1";
                }

                core::speed::Option opt = 0;
                if (!dp.empty())
                {
                    opt |= core::speed::PITCH;
                }
                if (!dt.empty())
                {
                    opt |= core::speed::TEMPO;
                }

                if (dryRun_)
                {
                    if (!quiet_)
                    {
                        std::cout << fmt::format("  [试运行] {}  变速={}  变调={} -> {}",
                                                 path.filename().string(),
                                                 dt.empty() ? "-" : dt,
                                                 dp.empty() ? "-" : dp,
                                                 output_.string())
                                  << std::endl;
                    }
                    ++success;
                    continue;
                }

                if (!quiet_)
                {
                    std::cout << fmt::format("[{}/{}] 处理中：{}", index, paths.size(), path.filename().string()) << std::endl;
                }

                services::SpeedRequest req;
                req.beatmapPath = core::fs::absolute(path);
                req.outputDir = output_;
                req.tempo = dt;
                req.pitch = dp;
                req.mode = opt;
                service.Process(req);

                if (!quiet_)
                {
                    std::cout << Green("  完成") << std::endl;
                }
                ++success;
            }
            catch (const core::OutilError &e)
            {
                ++failed;
                failures.emplace_back(path.filename().string(), e.what());
                std::cerr << Red(fmt::format("[{}/{}] 失败：{}", index, paths.size(), path.filename().string())) << std::endl;
                std::cerr << "  " << e.what() << std::endl;
            }
            catch (const std::exception &e)
            {
                ++failed;
                failures.emplace_back(path.filename().string(), e.what());
                std::cerr << Red(fmt::format("[{}/{}] 失败：{}", index, paths.size(), path.filename().string())) << std::endl;
                std::cerr << "  " << e.what() << std::endl;
            }
        }

        //  汇总
        if (!quiet_)
        {
            std::cout << std::endl;
            std::cout << Cyan("== 处理完成 ==") << std::endl;
        }
        std::cout << fmt::format("成功：{}  跳过：{}  失败：{}", success, skipped, failed) << std::endl;

        if (!failures.empty())
        {
            std::cerr << Yellow("失败详情：") << std::endl;
            for (const auto &[name, reason] : failures)
            {
                std::cerr << "  - " << name << "：" << reason << std::endl;
            }
        }

        if (failed > 0)
        {
            return core::EXIT_PARTIAL_FAILURE;
        }
        return core::EXIT_OK;
    }
} // namespace cli
