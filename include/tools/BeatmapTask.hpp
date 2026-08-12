/**
 * @file    ProcessTools.hpp
 * @brief   谱面倍速处理工具的异步派生类
 *
 * 该模块提供了异步调用的
 * 谱面处理实现，供 GUI 界面使用
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup tools
 */

#pragma once

#include "BeatmapTaskBase.hpp"
#include <QByteArray>
#include <QProcess>
#include <QString>

namespace tools
{
    /**
     * @brief 基于 QProcess 的异步谱面处理类
     *
     * 继承自 BeatmapTaskBase，重写了音频信息获取与音频处理逻辑
     */
    class BeatmapTask : public BeatmapTaskBase
    {
    private:
        SongInfo si_; // 歌曲元数据

        /**
         * @brief 异步获取音频采样率信息
         * @param errorMsg 失败时输出的错误信息
         * @return true 成功
         * @return false 失败
         */
        bool FetchAudioInfo(std::string *errorMsg = nullptr) override;

        /**
         * @brief 异步处理音频
         * @param outputPath 输出目录
         * @param errorMsg   失败时输出的错误信息
         * @return true 成功
         * @return false 失败
         */
        bool ProcessAudio(const core::fs::path &outputPath, std::string *errorMsg = nullptr) const override;

        /**
         * @brief 解析 ffprobe 返回的 JSON 输出
         * @param json ffprobe 的 JSON 输出
         * @return true 解析成功
         * @return false 解析失败
         */
        bool ParseFFprobeOutput(const QString json);

    public:
        using BeatmapTaskBase::BeatmapTaskBase;

        /**
         * @brief 执行完整的异步处理流程并输出到新目录
         *
         * @param newdir 输出到的目录
         */
        void Parse(const core::fs::path &newdir) override;
    };
}