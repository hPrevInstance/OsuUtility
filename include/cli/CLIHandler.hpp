/**
 * @file    CLIHandler.hpp
 * @brief   CLI 入口
 *
 * 提供命令行界面的启动函数。当用户带命令行参数运行程序时，
 * 由 main 调用本文件的 RunCLI 进入命令行模式。
 *
 * 主要流程：
 *  1. 创建顶层 CLI11 应用并注册各功能子命令
 *  2. 解析命令行参数
 *  3. 分发到被用户选中的子命令执行
 *
 * 当前子命令：
 *  - speed : 批量调整 .osu 谱面与音频的变速/变调
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup cli
 */

#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace cli
{
    //  常量
    inline constexpr const char *kProgramName = "outil";
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
} // namespace cli

int RunCLI(int argc, char **argv);
