/**
 * @file    FileOperation.hpp
 * @brief   谱面文件导出模块
 *
 * 提供与文件系统交互的辅助函数（导出文件）。
 * 谱面解析见 BeatmapParser，序列化见 BeatmapSerializer，
 * 倍速变换见 core/speed/SpeedTransform。
 *
 * @author  hPrevInstance
 * @date    2026-08-11
 * @version 1.2.0
 * @ingroup core
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "core/Fs.hpp"

#include <string>
#include <vector>

namespace core
{
    /**
     * @brief 将处理好的文件导出到 newdir，如果不存在则自动创建
     *
     * @param newdir 新目录名称
     * @param name 文件名
     * @param content 已处理的文件内容
     */
    void ExportFile(const fs::path &newdir, const fs::path &name, const std::vector<std::string> &content);
}
