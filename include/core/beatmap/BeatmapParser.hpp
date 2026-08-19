/**
 * @file    BeatmapParser.hpp
 * @brief   osu 谱面文件解析器
 *
 * 从输入流中逐行解析 osu 谱面文件，按章节填充 Beatmap 数据模型。
 * 解析逻辑与 I/O 分离：Parse 只依赖 std::istream，便于单元测试。
 *
 * @author  hPrevInstance
 * @version 1.2.0
 * @ingroup core
 */

#pragma once

#include "core/Fs.hpp"
#include "core/beatmap/Beatmap.hpp"

#include <istream>

namespace core
{
    /**
     * @brief 谱面解析器
     *
     * 负责把 osu 谱面文本解析为 Beatmap 数据模型。
     */
    class BeatmapParser
    {
    public:
        /**
         * @brief 从输入流解析谱面
         *
         * 逐行扫描，依据章节标题将各行分发到对应的解析器中。
         * 任何解析失败或无法识别的行都会被存入 Unknown_。
         *
         * @param in 输入流
         * @return Beatmap 解析完成的谱面对象
         */
        static Beatmap Parse(std::istream &in);

        /**
         * @brief 从文件路径解析谱面
         *
         * @param filename 谱面文件路径
         * @return Beatmap 解析完成的谱面对象
         * @throw OutilError 当文件无法打开时抛出
         */
        static Beatmap ParseFile(const fs::path &filename);
    };
}
