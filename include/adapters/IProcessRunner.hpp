/**
 * @file    IProcessRunner.hpp
 * @brief   外部进程执行抽象接口
 *
 * 把 ffmpeg/ffprobe 等外部工具的调用抽象为可注入的接口，
 * 便于替换实现（QProcess / popen / 测试 mock）并解耦服务层与具体进程实现。
 *
 * @ingroup adapters
 */

#pragma once

#include <string>
#include <vector>

namespace adapters
{
    /**
     * @brief 一次进程执行的结果
     */
    struct ProcessResult
    {
        int exitCode = -1;   // 进程退出码
        std::string stdOut;  // 标准输出
        std::string stdErr;  // 标准错误

        /// @brief 是否成功（退出码为 0）
        bool Succeeded() const { return exitCode == 0; }
    };

    /**
     * @brief 外部进程执行器接口
     *
     * 同步执行一个外部程序并收集其输出。
     * 异步与取消由上层（服务线程 / 取消令牌）负责。
     */
    class IProcessRunner
    {
    public:
        virtual ~IProcessRunner() = default;

        /**
         * @brief 同步执行外部程序
         *
         * @param program 可执行程序名（依赖 PATH）
         * @param args    参数列表（不经过 shell，每个元素为一个参数）
         * @return ProcessResult 退出码与输出
         */
        virtual ProcessResult Run(const std::string &program, const std::vector<std::string> &args) = 0;
    };
}
