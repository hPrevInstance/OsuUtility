/**
 * @file    Difficulty.hpp
 * @brief   osu谱面文件解析类
 *
 * 该模块提供了 Difficulty 类，负责逐行读取 osu 谱面文件，
 * 并按章节将其内容解析到对应的数据结构中。
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup core
 * @see     https://osu.ppy.sh/wiki/zh/Client/File_formats/osu_(file_format)
 */

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace core
{
    namespace fs = std::filesystem;

    /**
     * @brief 表示一个 osu 谱面对象
     *
     * 该类负责读取并存储一个 .osu 谱面文件被解析后的全部内容，
     * 供后续处理使用。
     *
     * 内部按 osu 文件格式的章节结构维护：
     *  - 键值型章节General/Editor/Metadata/Difficulty/Colors存于 map 中
     *  - 列表型章节Events/TimingPoints/HitObjects存于 vector 中
     *  - 无法识别的行统一存放到 Unknown_
     */
    class Difficulty
    {
    public:
        /**
         * @brief 谱面事件
         */
        struct Event
        {
            std::string type;              // 事件类型
            int start_time;                // 事件开始时间
            std::vector<std::string> args; // 事件附加参数
        };

        /**
         * @brief 时间点
         */
        struct TimingPoint
        {
            double time,      // 时间点位置
                beat_length;  // 节拍长度
            int meter,        // 每小节拍数
                sound_effect, // 音效类型
                sound_arg,    // 音效附加参数
                volume;       // 音量
            bool inherit;     // 是否为继承时间点
            int effect;       // 效果标志
        };

        /**
         * @brief 打击物件
         */
        struct Object
        {
            int x, y,                      // 物件在游玩区域中的坐标
                time,                      // 物件出现时间
                type,                      // 物件类型位掩码
                beat_sound;                // 音效编号
            std::vector<std::string> args, // 具体参数
                sound_group;               // 采样组参数，以 ':' 分隔
        };

    private:
        /**
         * 以下为键值型章节数据：
         *  key   —— 章节内条目名
         *  value —— 条目值
         */
        std::map<std::string, std::string>
            General_, Editor_, Metadata_, Difficulty_, Colors_;

        std::vector<Event> Events_;             // 事件列表
        std::vector<TimingPoint> TimingPoints_; // 时间点列表
        std::vector<Object> Objects_;           // 打击物件列表
        std::vector<std::string> Unknown_;      // 未能识别的原始行
        std::string version;                    // 文件头
        fs::path filename_;                     // 谱面文件路径
        std::vector<std::string> order_;        // 各键值章节中条目的出现顺序

    public:
        Difficulty() = default;
        /** @brief 以指定文件路径构造*/
        Difficulty(fs::path filename) : filename_(filename) {}
        Difficulty(const Difficulty &) = default;
        Difficulty(Difficulty &&) = default;
        ~Difficulty() = default;

        /**
         * @brief 读取并解析整个谱面文件
         *
         * 逐行扫描文件，依据章节标题将各行的内容分发到对应的解析器中。
         * 开头包含 "osu file format" 的行会被识别为版本信息。
         * 任何解析失败或无法识别的行都会被存入 Unknown_
         *
         * @throw std::invalid_argument 当文件无法打开时抛出
         */
        void load();
        /**
         * @name 键值型章节访问器
         * @brief 返回对应章节的键值映射
         * @return 对应章节的常量引用
         */
        const std::map<std::string, std::string> &GetGeneral() const { return General_; }
        const std::map<std::string, std::string> &GetEditor() const { return Editor_; }
        const std::map<std::string, std::string> &GetMetadata() const { return Metadata_; }
        const std::map<std::string, std::string> &GetDifficulty() const { return Difficulty_; }
        const std::map<std::string, std::string> &GetColors() const { return Colors_; }
        /// @brief 返回各键值章节中条目的出现顺序
        const std::vector<std::string> &GetOrder() const { return order_; }

        /**
         * @name 列表型章节访问器
         * @brief 返回对应章节的元素列表
         * @return 对应章节的常量引用
         */
        const std::vector<Event> &GetEvents() const { return Events_; }
        const std::vector<TimingPoint> &GetTimingPoints() const { return TimingPoints_; }
        const std::vector<Object> &GetObjects() const { return Objects_; }
        /// @brief 返回未被识别、原样保留的行
        const std::vector<std::string> &GetUnknownLines() const { return Unknown_; }

        /// @brief 返回谱面文件路径
        fs::path GetFilename() const { return filename_; }
        /// @brief 返回谱面文件版本
        std::string GetVersion() const { return version; }
    };
}