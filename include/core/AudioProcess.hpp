/**
 * @file AudioProcess.hpp
 * @author hPrevInstance
 * @brief 音频倍速参数处理模块
 *
 * 该模块提供了用于精确处理有理数倍速的函数，同时实现了较低的资源占用和较高的速度，
 * 还包含一个构造ffmpeg参数的处理函数，用于生成倍速处理的滤镜链。
 *
 * @version 1.0.0
 * @date 2026-08-03
 * @ingroup core
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

/**
 * @defgroup core 倍速参数处理模块
 * @brief 用于倍速处理
 */
namespace core
{
    /**
     * @brief 将有理数化简为a/b的最简分数形式
     *
     * @param speed 输入字符串，形式上只能为小数或分数
     * @return std::pair<int64_t, int64_t> first为最简分数的分子，second为分母
     * @throw std::invalid_argument 当输入字符串中含有不可识别的标记时
     * @throw std::out_of_range 当输入字符串表示的数超过任何整数类型时
     * @throw std::domain_error 当分母为0时
     */
    std::pair<int64_t, int64_t> ParseFraction(const std::string &speed);

    /**
     * @brief 将倍速分解为[1/2, 2]内的因子且因子最少
     *
     * @param speed 要分解的有理数，分子在first中，分母在second中
     * @return std::vector<std::string> 分解后的因子序列
     *
     * @note 本函数只接受64位整数，若超出范围则行为不可控
     */
    std::vector<std::string> Factoring(const std::pair<int64_t, int64_t> &speed);

    typedef uint8_t Option;
    inline constexpr Option TEMPO = 0b1;
    inline constexpr Option PITCH = 0b10;

    /**
     * @brief 生成ffmpeg的变速变调参数
     *
     * @param tempo 变速速度
     * @param pitch 变调速度
     * @param sampleRate ffprobe返回的变调采样率
     * @param opt 选择是变速还是变调，或两者都是
     * @return std::string 生成的参数
     * @throw std::invalid_argument 采样率小于等于0时
     */
    std::string GenerateFilterChain(const std::string &tempo, const std::string &pitch, int sampleRate, Option opt);
}