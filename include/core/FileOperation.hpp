/**
 * @file    FileOperation.hpp
 * @brief   osu谱面文件解析与倍速调整核心模块
 *
 * 该模块提供了完整的 osu谱面文件读写、解析和倍速缩放功能。
 * 支持处理General\Events\TimingPoints\HitObjects章节中的时间相关字段，
 * 并包含一个通用的分隔字段解析器。
 *
 * @author  hPrevInstance
 * @date    2026-07-30
 * @version 1.0.0
 * @ingroup core
 *
 * @see     https://osu.ppy.sh/wiki/zh/Client/File_formats/osu_(file_format) 中文官方谱面格式文档
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <fmt/format.h>

/**
 * @defgroup core 核心算法模块
 * @brief 包含所有与界面无关的文件处理与谱面解析逻辑
 */
namespace core
{
    namespace fs = std::filesystem;
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
         * @param std::string 传入要解析的字符串
         * @param std::string 分隔符集合
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
                return line.substr(beg, Size()); // 返回字段
            }
            return std::string{};
        }
        /**
         * @brief 当前处理了多少个字段
         *
         * @return size_t
         */
        size_t Count()
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
        void Reset(size_t relocate = 0ULL)
        {
            this->isValid = true;
            this->beg = 0;
            this->end = 0;
            this->cnt = 0;
            Ignore(relocate);
        }
        /**
         * @brief 跳过n个字段
         *
         * @param n
         */
        void Ignore(size_t n)
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
        size_t Begin() const
        {
            return beg;
        }
        /**
         * @brief 返回当前字段的长度，初始值为0
         *
         * @return size_t
         */
        size_t Size() const
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
    /**
     * @brief 判断一个字符串是否以换行符结尾
     *
     * @param s 字符串
     * @return true 是
     * @return false 否
     */
    inline bool EndsWithNewline(const std::string &s)
    {
        if (s.empty())
        {
            return false;
        }
        // 检查常见的换行符序列
        if (s.back() == '\n')
        {
            // 如果末尾是 \n，再检查它前面是否是 \r
            if (s.size() >= 2 && s[s.size() - 2] == '\r')
            {
                return true; // 匹配 \r\n
            }
            return true; // 匹配 \n
        }
        return false;
    }

    /**
     * @brief 加载指定的osu谱面文件
     *
     * @param filename 该文件的路径
     * @return std::vector<std::string> 分割为字符串向量的osu文件
     * @throw std::invaild_argument 当不是osu文件或打开失败时抛出
     */
    inline std::vector<std::string> LoadBeatmapFile(const fs::path &filename)
    {
        if (filename.extension() != ".osu")
        {
            throw std::invalid_argument("参数错误：参数不是osu谱面文件：" + filename.string());
        }
        std::ifstream ifs(filename);
        if (!ifs)
        {
            throw std::runtime_error("系统错误：无法打开文件：" + filename.string());
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(ifs, line))
        {
            lines.push_back(line);
        }
        return lines;
    }
    /**
     * @brief 处理章节[beg,end)中的所有时间字段，将其缩放为指定倍速
     *
     * @tparam It 包含章节的迭代器类型
     * @tparam Inserter 输出迭代器类型
     * @param beg 章节起始位置，包含该章节的名称
     * @param end 结束位置
     * @param dist 处理结果的输出位置
     * @param speed 倍速
     * @param name 当前处理文件名，用于异常处理
     *
     * @note 为了简洁这里使用了模板参数，实际传参须指定为字符串向量类型的开区间和输出位置
     */
    template <typename It, typename Inserter>
    inline void ProcessSection(const It beg, const It end, Inserter dist, long double speed, const fs::path &name)
    {
        std::string sec = *beg;
        if (beg == end)
        {
            return;
        }
        *dist = sec; // 存入章节名
        size_t row = 0;
        for (auto b = beg + 1; b != end; ++b)
        {
            row++;
            std::string str = *b;
            PunctLexer cl(str, ",:");
            try
            {
                // 忽略换行符
                if (str.empty() || !cl || EndsWithNewline(str))
                {
                    *dist = str;
                    continue;
                }
                // 谱面基本信息
                if (sec.find("[General]") != std::string::npos)
                {
                    // 只处理预览时间字段，因为该章节有关字段仅此一个，即PreviewTime:...
                    if (str.find("PreviewTime") != std::string::npos)
                    {
                        cl.Ignore(1);
                        uint32_t msec = std::stoul(cl.NextField()); // 毫秒坐标
                        msec = static_cast<uint32_t>(msec / speed); // osu要求该字段必须为整型
                        str.replace(cl.Begin(), cl.Size(), " " + fmt::format("{}", msec));
                    }
                    *dist = str; // 其他字段原样返回
                }
                // 元数据
                else if (sec.find("[Metadata]") != std::string::npos)
                {
                    if (str.find("Version") != std::string::npos)
                    {
                        if (EndsWithNewline(str))
                        {
                            str.erase(str.end() - 1);
                        }
                        str.append(fmt::format(" {:.10g}x\n", speed)); // 在游戏中更好分辨倍速
                    }
                    *dist = str; // 其他字段原样返回
                }
                // 事件
                else if (sec.find("[Events]") != std::string::npos)
                {
                    cl.Ignore(1); // 跳过第一个
                    std::string front = cl.NextField();
                    // 视频，第二个字段为时间
                    if (front == "1" || front == "Video")
                    {
                        uint32_t msec = std::stoul(cl.NextField());
                        msec = static_cast<uint32_t>(msec / speed);
                        str.replace(cl.Begin(), cl.Size(), fmt::format("{}", msec)); // 不包含逗号
                    }
                    // 休息段，一个开始时间，一个结束时间
                    else if (front == "2" || front == "Break")
                    {
                        // 直接处理两次即可
                        for (int i = 0; i < 2; i++)
                        {
                            uint32_t msec = std::stoul(cl.NextField());
                            msec = static_cast<uint32_t>(msec / speed);
                            str.replace(cl.Begin(), cl.Size(), fmt::format("{}", msec));
                        }
                        // 其他不含时间，故不处理
                        // 暂不支持故事板处理
                    }
                    *dist = str;
                }
                // 时间点
                else if (sec.find("[TimingPoints]") != std::string::npos)
                {
                    // 先处理时间点，官方要求为小数
                    long double time = std::stold(cl.NextField()) / speed;
                    auto cnt = cl.Count();
                    str.replace(cl.Begin(), cl.Size(), fmt::format("{:.10g}", time));
                    cl.Reset(cnt);
                    // 再处理拍长，同为小数
                    long double beatLen = std::stold(cl.NextField());
                    if (beatLen < 0)
                    {
                        // 为负数则表示继承点滑条速度，按照定义应乘以倍速
                        beatLen *= speed;
                    }
                    else
                    {
                        // 为正数则表示非继承点拍长，按倍速缩放
                        beatLen /= speed;
                    }
                    str.replace(cl.Begin(), cl.Size(), fmt::format("{:.10g}", beatLen));
                    *dist = str;
                }
                // 物件
                else if (sec.find("[HitObjects]") != std::string::npos)
                {
                    // 跳过x, y字段
                    cl.Ignore(2);
                    // 该字段为整型
                    uint32_t t = std::stoul(cl.NextField());
                    t = static_cast<uint32_t>(t / speed);
                    auto cnt = cl.Count();
                    str.replace(cl.Begin(), cl.Size(), fmt::format("{}", t));
                    cl.Reset(cnt);
                    // 获取该物件的类型
                    uint8_t type = std::stoi(cl.NextField());
                    // 如果为std滑条，则第2位为1
                    if (type & (1 << 1))
                    {
                        cl.Ignore(3); // 向后跳3位是长度
                    }
                    // 如果是mania长条或转盘，则4或8位为1
                    else if (type & (1 << 7) || type & (1 << 3))
                    {
                        cl.Ignore(1); // 向后跳1位是长度
                    }
                    // 普通物件
                    else if (type & 1)
                    {
                        *dist = str;
                        continue;
                    }
                    // 根据官方要求，这一位是整型
                    uint32_t len = static_cast<uint32_t>(std::stoul(cl.NextField()) / speed);
                    str.replace(cl.Begin(), cl.Size(), fmt::format("{}", len));
                    *dist = str;
                }
                else
                {
                    *dist = str;
                }
            }
            catch (const std::exception &e)
            {
                throw std::invalid_argument(fmt::format("格式错误：文件{} 章节{} {}行：{}\n", name.filename().string(), sec, row, e.what()));
            }
        }
    }
    /**
     * @brief 处理osu谱面文件的各个章节的时间倍速
     *
     * @param filename 文件路径，主要用于异常处理
     * @param lines 谱面的字符串向量
     * @param speed 倍速
     * @return std::vector<std::string> 返回处理后的文件向量
     * @throw 当文件头不是官方指定或空文件时抛出
     */
    inline std::vector<std::string> ProcessBeatmap(const fs::path &filename, const std::vector<std::string> &lines, long double speed)
    {
        std::vector<std::string> processed;
        if (lines.empty() || lines.front().find("osu file format") == std::string::npos)
        {
            throw std::invalid_argument(fmt::format("格式错误：该osu谱面文件无效：{}\n", filename.string()));
        }
        processed.reserve(lines.size());       // 预分配内存
        processed.emplace_back(lines.front()); // osu文件头
        processed.emplace_back("\r");

        // 寻找章节[...]的lambda，为简洁不使用正则表达式
        auto findSec = [](const std::string &s)
        { return !s.empty() && s.find('[') != std::string::npos && s.find(']') != std::string::npos; };

        auto secBeg = std::find_if(lines.begin(), lines.end(), findSec); // 起始
        auto secEnd = std::find_if(secBeg + 1, lines.end(), findSec);    // 结束
        auto inserter = std::back_inserter(processed);                   // 插入器

        while (secBeg != lines.end())
        {
            ProcessSection(secBeg, secEnd, inserter, speed, filename);
            if (secEnd == lines.end())
            {
                break;
            }
            // 处理下一个章节
            secBeg = secEnd;
            secEnd = std::find_if(secBeg + 1, lines.end(), findSec);
        }
        return processed;
    }
    /**
     * @brief 将处理好的文件导出到newdir，如果不存在则自动创建
     *
     * @param newdir 新目录名称
     * @param name 文件名
     * @param content 已处理的文件内容
     */
    inline void ExportFile(const fs::path &newdir, const fs::path &name, const std::vector<std::string> &content)
    {
        fs::create_directories(newdir);
        std::ofstream ofs(newdir / name.filename());
        for (auto &line : content)
        {
            ofs << line << '\r';
        }
    }
}