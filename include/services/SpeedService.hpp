/**
 * @file    SpeedService.hpp
 * @brief   倍速服务
 *
 * 编排 core 纯算法与 adapters 进程适配，完成单个谱面的倍速处理流程：
 *  解析 -> 变换 -> ffprobe 采样率 -> 导出谱面 -> ffmpeg 处理音频 -> 复制背景图。
 *
 * 服务层不依赖任何具体 Qt Widget，GUI 与 CLI 共用本服务。
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup services
 */

#pragma once

#include "adapters/IProcessRunner.hpp"
#include "core/Fs.hpp"
#include "core/speed/AudioProcess.hpp"
#include "services/Progress.hpp"

#include <functional>
#include <string>

namespace services
{
    /**
     * @brief 一次倍速处理的请求参数
     */
    struct SpeedRequest
    {
        core::fs::path beatmapPath; // 源谱面路径
        core::fs::path outputDir;   // 输出目录
        std::string tempo;          // 变速
        std::string pitch;          // 变调
        core::speed::Option mode = 0; // 处理模式（TEMPO/PITCH 位掩码）
    };

    /**
     * @brief 倍速处理服务
     *
     * 通过构造函数注入外部进程执行器，便于测试与替换实现。
     */
    class SpeedService
    {
    public:
        /// @brief 进度回调，返回 false 表示请求取消
        using ProgressCb = std::function<bool(const Progress &)>;

        /**
         * @param runner 外部进程执行器（注入）
         */
        explicit SpeedService(adapters::IProcessRunner &runner) : runner_(runner) {}

        /**
         * @brief 同步处理单个谱面
         *
         * @param req        请求参数
         * @param onProgress 进度回调（可空）
         * @return core::fs::path 输出的谱面路径
         * @throw core::OutilError 处理失败或用户取消时抛出
         */
        core::fs::path Process(const SpeedRequest &req, ProgressCb onProgress = {}) const;

    private:
        adapters::IProcessRunner &runner_;
    };
}
