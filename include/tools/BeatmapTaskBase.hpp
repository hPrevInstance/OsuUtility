/**
 * @file    BeatmapTaskBase.hpp
 * @brief   谱面倍速处理工具的同步基类
 *
 * 该模块提供了谱面处理流程的串行实现：读取谱面信息、调用 ffprobe 获取
 * 音频采样率、处理谱面文件文本、调用 ffmpeg 处理音频并导出结果。
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup tools
 */

#pragma once

#include "core/AudioProcess.hpp"
#include "core/FileOperation.hpp"
#include <string>

/**
 * @brief 谱面处理工具
 *
 * @defgroup tools
 */
namespace tools
{
    /**
     * @brief  存放歌曲元数据
     *
     * 音频文件的元信息
     */
    class SongInfo
    {
    };

    /**
     * @brief 谱面处理基类，用于CLI同步处理
     *
     * 封装了一次完整的谱面倍速处理流程所需的全部状态与方法：
     *  - 谱面、音频、背景图的路径
     *  - 变速/变调参数与模式
     *  - 音频信息获取与音频重采样处理
     */
    class BeatmapTaskBase
    {
    protected:
        core::fs::path filename_, song_, picture_; // 谱面、歌曲路径、背景图
        std::string tempo_, pitch_;                // 变速变调
        core::Option opt_ = 0;                     // 选项
        core::Difficulty diff_;                    // 已解析的谱面对象

        /**
         * @brief 获取音频的采样率信息
         *
         * 通过调用 ffprobe 读取音频流采样率并保存到 sampleRate_。
         * 派生类可覆写以实现不同的实现方式。
         *
         * @param errorMsg 失败时输出的错误信息
         * @return true 成功
         * @return false 失败
         */
        virtual bool FetchAudioInfo(std::string *errorMsg);

        /**
         * @brief 处理谱面音频
         *
         * @param newdir   输出目录
         * @param errorMsg 失败时输出的错误信息
         * @return true 成功
         * @return false 失败
         */
        virtual bool ProcessAudio(const core::fs::path &newdir, std::string *errorMsg) const;

    private:
        int sampleRate_; // 音频采样率

    public:
        /**
         * @brief 处理器构造函数
         *
         * @param bmp 谱面的路径
         */
        BeatmapTaskBase(const core::fs::path &p) : diff_(p)
        {
            filename_ = p;
            diff_.load();
        }
        BeatmapTaskBase(const BeatmapTaskBase &) = default;
        BeatmapTaskBase(BeatmapTaskBase &&) = default;

        /// @brief 设置处理模式
        void SetMode(core::Option opt)
        {
            opt_ = opt;
        }
        /// @brief 设置变调参数
        void SetPitch(const std::string &p)
        {
            pitch_ = p;
        }
        /// @brief 设置变速参数
        void SetTempo(const std::string &t)
        {
            tempo_ = t;
        }
        /**
         * @brief 执行完整的处理流程并输出到新目录
         *
         * @param newdir 输出到的目录
         */
        virtual void Parse(const core::fs::path &newdir);
        virtual ~BeatmapTaskBase() = default;
    };
}