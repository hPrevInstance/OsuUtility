/**
 * @file    AudioProcess.cpp
 * @brief   音频倍速参数处理模块实现
 * @ingroup core
 */

#include "core/AudioProcess.hpp"
#include "core/Error.hpp"

#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>

namespace core
{
    std::pair<int64_t, int64_t> ParseFraction(const std::string &speed)
    try
    {
        size_t div = 0;    // 分数线或小数点
        int64_t num = 0;   // 分子
        int64_t denom = 1; // 分母
        if (speed.empty())
        {
            throw MakeParamError("倍速参数不应为空");
        }
        if ((div = speed.find('/')) != std::string::npos)
        {
            num = std::stoll(speed.substr(0, div));    // 分子
            denom = std::stoll(speed.substr(div + 1)); // 分母
            if (denom == 0)
            {
                throw MakeParamError("分母不应为 0");
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
    catch (const std::invalid_argument &ia)
    {
        throw MakeParamError(fmt::format("倍速中含有非数字成分：{}", ia.what()));
    }
    catch (const std::out_of_range &o)
    {
        throw MakeParamError(fmt::format("倍速过大或精度过高：{}", o.what()));
    }

    std::vector<std::string> Factoring(const std::pair<int64_t, int64_t> &speed)
    {
        auto &[a, b] = speed;
        if (a <= 0 || b <= 0)
        {
            throw MakeParamError("倍速不应小于等于 0");
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

    std::string GenerateFilterChain(const std::string &tempo, const std::string &pitch, int sampleRate, Option opt)
    {
        if (sampleRate <= 0)
        {
            throw MakeParamError("采样率不应小于等于 0");
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
