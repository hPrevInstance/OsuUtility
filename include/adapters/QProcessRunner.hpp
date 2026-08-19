/**
 * @file    QProcessRunner.hpp
 * @brief   基于 QProcess 的外部进程执行器
 *
 * IProcessRunner 的 Qt 实现，使用 QProcess 同步执行外部程序。
 *
 * @ingroup adapters
 */

#pragma once

#include "adapters/IProcessRunner.hpp"

namespace adapters
{
    /**
     * @brief 使用 QProcess 同步执行外部程序
     */
    class QProcessRunner : public IProcessRunner
    {
    public:
        ProcessResult Run(const std::string &program, const std::vector<std::string> &args) override;
    };
}
