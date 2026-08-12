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

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif

#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <fmt/format.h>

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
    inline std::pair<int64_t, int64_t> ParseFraction(const std::string &speed)
    try
    {
        size_t div = 0; // 分数线或小数点
        size_t num = 0, denom = 1;
        if (speed.empty())
        {
            throw std::invalid_argument("参数不应为空");
        }
        if ((div = speed.find('/')) != std::string::npos)
        {
            num = std::stoll(speed.substr(0, div));    // 分子
            denom = std::stoll(speed.substr(div + 1)); // 分母
            if (denom == 0)
            {
                throw std::domain_error("分母不应为0");
            }
        }
        else if ((div = speed.find('.')) != std::string::npos)
        {
            num = std::stoll(speed.substr(0, div)); // 整数部分
            if (speed.front() == '.')
            {
                num = 0;
            }

            std::string fracpart(1, '0'); // 小数部分
            if (speed.back() != '.')
            {
                fracpart = speed.substr(div + 1);
            }
            // 快速幂
            auto fastexp = [](long long a, long long b) -> long long
            {
                long long res = 1;
                while (b)
                {
                    if (b & 1)
                    {
                        res *= a;
                    }
                    a *= a;
                    b >>= 1;
                }
                return res;
            };
            denom = fastexp(10, fracpart.size());
            num = num * denom + std::stoll(fracpart);
        }
        else
        {
            num = std::stoll(speed);
        }
        auto g = std::gcd(num, denom);
        return std::make_pair(num / g, denom / g);
    }
    catch (std::invalid_argument &ia)
    {
        throw std::invalid_argument(fmt::format("参数错误：倍速中含有非数字成分：{}\n", ia.what()));
    }
    catch (std::out_of_range &o)
    {
        throw std::invalid_argument(fmt::format("参数错误：倍速过大或精度过高：{}\n", o.what()));
    }

    /**
     * @brief 将倍速分解为[1/2, 2]内的因子且因子最少
     *
     * @param speed 要分解的有理数，分子在first中，分母在second中
     * @return std::vector<std::string> 分解后的因子序列
     *
     * @note 本函数只接受64位整数，若超出范围则行为不可控
     */
    inline std::vector<std::string> Factoring(const std::pair<int64_t, int64_t> &speed)
    {
        auto &[a, b] = speed;
        if (a <= 0 || b <= 0)
        {
            throw std::invalid_argument(fmt::format("参数错误：倍速不应小于等于0\n"));
        }
        // q >= log2 max(s, s ^ -1) 且 max(s, s ^ -1) = max(a, b) / min(a, b), s 或 s ^ -1 = a / b
        int64_t A = std::max(a, b), B = std::min(a, b), q = 0;
        auto bitlen = [](int64_t x) -> int
        {
            int len = 0;
            while (x)
            {
                ++len;
                x >>= 1;
            }
            return len;
        }; // 计算二进制位长度
        if (A == B)
        {
            return {"1/1"};
        }
        else if (A > B)
        {
            // 2 ^ (q - 1) <= A / B <= 2 ^ q -> q = ceil(log2 A / B)
            auto d = bitlen(A) - bitlen(B);
            // q = d if A <= B * 2 ^ d else q = d + 1
            q = A <= B * (1ULL << d) ? d : d + 1;
        }
        // 贪心最优分解
        if (a > b)
        {
            std::vector<std::string> ret(q - 1, "2");
            ret.emplace_back(fmt::format("{}/{}", a, b * (1ULL << (q - 1))));
            return ret;
        }
        else
        {
            std::vector<std::string> ret(q - 1, "1/2");
            ret.emplace_back(fmt::format("{}/{}", a * (1ULL << (q - 1)), b));
            return ret;
        }
        return {};
    }

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
    inline std::string GenerateFilterChain(const std::string &tempo, const std::string &pitch, int sampleRate, Option opt)
    {
        if (sampleRate <= 0)
        {
            throw std::invalid_argument("采样率不应小于等于0");
        }
        if ((opt & TEMPO) && (opt & PITCH)) // 变速变调
        {
            std::string ret("-af \"");
            auto [num, denom] = ParseFraction(pitch);
            ret.append(fmt::format("asetrate={},", std::llround((double)num / denom * sampleRate)));
            auto DivFrac = [](const std::string &a, const std::string &b) -> std::string
            {
                auto f1 = ParseFraction(a), f2 = ParseFraction(b);
                int64_t num = f1.first * f2.second;
                int64_t den = f1.second * f2.first;
                long long g = std::gcd(num, den);
                return fmt::format("{}/{}", num / g, den / g);
            };
            auto factors = Factoring(ParseFraction(DivFrac(tempo, pitch)));
            for (const auto &fact : factors)
            {
                ret.append(fmt::format("atempo={},", fact));
            }
            ret.append(fmt::format("aresample={}\"", sampleRate));
            return ret;
        }
        else if (opt & TEMPO) // 变速不变调
        {
            std::string ret("-af \"");
            auto factors = Factoring(ParseFraction(tempo));
            for (const auto &fact : factors)
            {
                ret.append(fmt::format("atempo={},", fact));
            }
            ret.replace(ret.size() - 1, 1, "\"");
            return ret;
        }
        else if (opt & PITCH) // 变调不变速
        {
            std::string ret("-af \"");
            auto [num, denom] = ParseFraction(pitch);
            ret.append(fmt::format("asetrate={},", std::llround((double)num / denom * sampleRate)));
            auto togfact = Factoring({num, denom});
            for (const auto &fact : togfact)
            {
                // 取倒数
                auto slash = fact.find('/');
                auto num = fact.substr(0, slash);
                auto denom = fact.substr(slash + 1);
                ret.append(fmt::format("atempo={}/{},", denom, num));
            }
            ret.append(fmt::format("aresample={}\"", sampleRate));
            return ret;
        }
        return "";
    }
}