/**
 * @file    Utility.hpp
 * @brief   通用工具
 *
 * 提供与业务无关的通用工具：字符串 trim 与标点分隔文本解析器。
 *
 * @ingroup common
 */

#pragma once

#include <string>

namespace common
{
    /**
     * @brief 去除字符串首尾的空白字符
     *
     * @param s     待处理的字符串
     * @param chars 视为空白的字符集合
     * @return std::string 去除首尾空白后的新字符串
     */
    inline std::string trim(const std::string &s, const std::string &chars = " \t\n\r\f\v")
    {
        auto start = s.find_first_not_of(chars);
        auto end = s.find_last_not_of(chars);
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    /**
     * @brief 一个用于处理标点分隔的字符串的文本解析类
     */
    class PunctLexer
    {
        const std::string &line; // 要解析的字符串的引用，由于涉及外部状态改变，不使用std::string_view
        size_t beg = 0;          // 当前位置
        size_t end = 0;          // 当前字段尾后位置
        std::string puncts;      // 标点集合
        size_t cnt = 0;          // 已处理字段个数
        bool isValid = true;     // 解析器状态是否有效

    public:
        /**
         * @brief 解析类构造函数
         *
         * @param l   传入要解析的字符串
         * @param pun 分隔符集合
         */
        PunctLexer(const std::string &l, const std::string &pun = ",") : line(l), puncts(pun), isValid(!line.empty()) {}
        /**
         * @brief 获取下一个字段
         *
         * @return std::string 传入字符串下一个字段的拷贝
         */
        std::string NextField()
        {
            if (isValid) // 在可用状态
            {
                beg = cnt == 0 ? end : end + 1;        // 初始状态为0
                end = line.find_first_of(puncts, beg); // 分隔符位置
                cnt++;
                if (end == std::string::npos) // 已到达末尾
                {
                    isValid = false;
                    return line.substr(beg); // 最后一个
                }
                return line.substr(beg, size()); // 返回字段
            }
            return std::string{};
        }
        /**
         * @brief 当前处理了多少个字段
         *
         * @return size_t
         */
        size_t count()
        {
            return cnt;
        }
        /**
         * @brief 在引用的外部容器改变时重置当前状态
         *
         * @param relocate 需要重定位的位置
         *
         * @warning 一旦外部容器发生改变，必须调用此成员，否则可能导致未定义行为
         */
        void reset(size_t relocate = 0ULL)
        {
            this->isValid = true;
            this->beg = 0;
            this->end = 0;
            this->cnt = 0;
            ignore(relocate);
        }
        /**
         * @brief 跳过n个字段
         *
         * @param n
         */
        void ignore(size_t n)
        {
            for (size_t i = 0; i < n; i++)
            {
                NextField();
            }
        }
        /**
         * @brief 返回当前字段的开始位置
         *
         * @return size_t
         */
        size_t begin() const
        {
            return beg;
        }
        /**
         * @brief 返回当前字段的长度，初始值为0
         *
         * @return size_t
         */
        size_t size() const
        {
            return end - beg;
        }
        /**
         * @brief 类型转换重载运算符，表示该解析器是否有效
         *
         * @return bool 为真表示有效，反之则无效
         */
        operator bool() const noexcept
        {
            return isValid;
        }
    };
}
