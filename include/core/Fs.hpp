/**
 * @file    Fs.hpp
 * @brief   文件系统命名空间别名
 *
 * 统一提供 core::fs = std::filesystem 的别名，避免各模块重复声明。
 *
 * @ingroup core
 */

#pragma once

#include <filesystem>

namespace core
{
    namespace fs = std::filesystem;
}
