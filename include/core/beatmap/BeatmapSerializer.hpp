/**
 * @file    BeatmapSerializer.hpp
 * @brief   osu 谱面序列化器
 *
 * 把 Beatmap 数据模型序列化为 osu 文件格式的行序列，
 * 纯格式化逻辑，不含任何数据修改或 I/O 副作用。
 *
 * @author  hPrevInstance
 * @version 1.2.0
 * @ingroup core
 */

#pragma once

#include "core/beatmap/Beatmap.hpp"

#include <string>
#include <vector>

namespace core
{
    /**
     * @brief 谱面序列化器
     */
    class BeatmapSerializer
    {
    public:
        /**
         * @brief 将谱面序列化为 osu 文件的行
         *
         * @param beatmap 已处理的谱面对象
         * @return std::vector<std::string> osu 文件的每一行
         */
        static std::vector<std::string> ToLines(const Beatmap &beatmap);
    };
}
