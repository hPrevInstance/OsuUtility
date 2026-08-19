/**
 * @file    SpeedTransform.hpp
 * @brief   谱面倍速变换纯算法
 *
 * 从谱面处理流程中抽离出来的纯函数：对 Beatmap 数据模型做倍速变换，
 * 返回一个新的 Beatmap，并顺带抽取音频文件名与背景图文件名。
 * 不涉及任何文件读写或外部进程调用。
 *
 * @author  hPrevInstance
 * @version 1.2.0
 * @ingroup core
 */

#pragma once

#include "core/beatmap/Beatmap.hpp"

#include <string>

namespace core::speed
{
    /**
     * @brief 谱面中引用的媒体文件信息
     */
    struct MediaInfo
    {
        std::string audio;       // General 章节的 AudioFilename
        std::string background;  // Events 章节中 type=0 事件的背景图文件名
    };

    /**
     * @brief 对谱面数据做倍速变换
     *
     * 变换内容包括：
     *  - General.PreviewTime 按倍速缩放
     *  - Metadata.Version 追加倍速标记，BeatmapID/BeatmapSetID 置 0
     *  - Events 开始时间与 break 结束时间按倍速缩放
     *  - TimingPoints 时间与继承点的节拍长度按倍速缩放
     *  - HitObjects 出现时间与滑条/转盘结束时间按倍速缩放
     *
     * @param src   源谱面
     * @param speed 倍速（> 0）
     * @param media 输出：抽取到的音频/背景图文件名
     * @return Beatmap 变换后的新谱面
     */
    Beatmap Transform(const Beatmap &src, double speed, MediaInfo &media);
}
