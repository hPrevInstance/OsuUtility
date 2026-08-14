/**
 * @file    CLIHandler.hpp
 * @brief   CLI 入口
 *
 * 提供命令行界面的启动函数。当用户带命令行参数运行程序时，
 * 由 main 调用本文件的 RunCLI 进入命令行模式。
 *
 * 主要流程：
 *  1. 用 CLI11 解析命令行选项
 *  2. 收集待处理的谱面文件列表
 *  3. 逐一对每个 .osu 谱面执行倍速处理，输出进度与统计
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup cli
 */

#pragma once

#include "CLI11.hpp"
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace cli
{
    //  常量
    inline constexpr const char *kProgramName = "osp";
    inline constexpr const char *kVersion = "1.0.0";

    //  颜色输出
    inline bool g_useColor = false;

    /**
     * @brief 使用 ANSI 转义序列着色，未启用颜色时原样返回
     */
    inline std::string Colorize(const char *code, const std::string &text)
    {
        if (!g_useColor)
        {
            return text;
        }
        return std::string("\033[") + code + "m" + text + "\033[0m";
    }
    inline std::string Red(const std::string &t) { return Colorize("31", t); }
    inline std::string Green(const std::string &t) { return Colorize("32", t); }
    inline std::string Yellow(const std::string &t) { return Colorize("33", t); }
    inline std::string Cyan(const std::string &t) { return Colorize("36", t); }
    inline std::string Dim(const std::string &t) { return Colorize("2", t); }

    /**
     * @brief 自动检测是否应启用颜色输出
     */
    inline bool DetectColor()
    {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        return GetFileType(hOut) == FILE_TYPE_CHAR;
#else
        return isatty(fileno(stdout)) != 0;
#endif
    }

    //  速度校验
    /**
     * @brief 校验速度字符串，返回空串表示合法，否则返回错误信息
     */
    std::string ValidateSpeed(const std::string &input);

    /**
     * @brief 供 CLI11 使用的速度校验器
     */
    inline CLI::Validator SpeedValidator()
    {
        return CLI::Validator(
            [](std::string &value) -> std::string
            { return ValidateSpeed(value); },
            "倍速，小数或分数，如 1.5、3/2",
            "SPEED");
    }

    //  映射文件条目
    /**
     * @brief 每谱面不同速度的条目
     */
    struct DiffEntry
    {
        std::string tempo; // 变速
        std::string pitch; // 变调
    };
} // namespace cli

int RunCLI(int argc, char **argv);
