/**
 * @file    Progress.hpp
 * @brief   服务层通用进度结构
 *
 * 供各服务（倍速/转谱/反键/KPS 等）向上层（GUI / CLI）上报进度。
 *
 * @ingroup services
 */

#pragma once

#include <string>

namespace services
{
    /**
     * @brief 处理进度
     */
    struct Progress
    {
        int percent = -1;    // 0~100；-1 表示阶段不确定
        std::string message; // 当前阶段描述
    };
}
