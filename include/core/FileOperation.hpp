/**
 * @file    FileOperation.hpp
 * @brief   osu谱面文件解析与倍速调整核心模块
 *
 * 该模块提供了完整的 osu谱面文件读写和倍速缩放功能。
 *
 * @author  hPrevInstance
 * @date    2026-08-11
 * @version 1.0.0
 * @ingroup core
 *
 * @see     https://osu.ppy.sh/wiki/zh/Client/File_formats/osu_(file_format)
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "Difficulty.hpp"

/**
 * @defgroup core 核心算法模块
 * @brief 包含所有与界面无关的文件处理与谱面解析逻辑
 */
namespace core
{
    namespace fs = std::filesystem;

    /**
     * @brief 加载指定的osu谱面文件
     *
     * @param filename 该文件的路径
     * @return std::vector<std::string> 分割为字符串向量的osu文件
     * @throw std::invaild_argument 当不是osu文件或打开失败时抛出
     */
    std::vector<std::string> LoadBeatmapFile(const fs::path &filename);
    /**
     * @brief 处理osu谱面文件的各个章节的时间倍速
     *
     * @param bmp 谱面对象
     * @param speed 倍速
     * @param exinfo 处理过程的额外信息
     * @return std::vector<std::string> 返回处理后的文件向量
     * @throw 当文件头不是官方指定或空文件时抛出
     */
    std::vector<std::string>
    ProcessBeatmap(const Difficulty &bmp,
                   std::vector<std::pair<std::string, std::string>> &exinfo,
                   double speed);
    /**
     * @brief 将处理好的文件导出到newdir，如果不存在则自动创建
     *
     * @param newdir 新目录名称
     * @param name 文件名
     * @param content 已处理的文件内容
     */
    void ExportFile(const fs::path &newdir, const fs::path &name, const std::vector<std::string> &content);
}