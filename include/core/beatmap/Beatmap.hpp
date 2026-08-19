/**
 * @file    Beatmap.hpp
 * @brief   osu 谱面数据模型
 *
 * 纯数据模型，只负责存储一份已解析的谱面内容，不含任何 I/O 逻辑：
 *  - 解析逻辑见 BeatmapParser
 *  - 序列化逻辑见 BeatmapSerializer
 *  - 倍速变换等纯算法见 core/speed
 *
 * @author  hPrevInstance
 * @version 1.2.0
 * @ingroup core
 * @see     https://osu.ppy.sh/wiki/zh/Client/File_formats/osu_(file_format)
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace core
{
    class BeatmapParser; // 前置声明：解析器需要增量填充本类的私有成员

    /**
     * @brief 表示一个 osu 谱面对象
     *
     * 内部按 osu 文件格式的章节结构维护：
     *  - 键值型章节General/Editor/Metadata/Difficulty/Colors存于 map 中
     *  - 列表型章节Events/TimingPoints/HitObjects存于 vector 中
     *  - 无法识别的行统一存放到 Unknown_
     */
    class Beatmap
    {
        friend class BeatmapParser;

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
        std::map<std::string, std::string>
            General_, Editor_, Metadata_, Difficulty_, Colors_;

        std::vector<Event> Events_;             // 事件列表
        std::vector<TimingPoint> TimingPoints_; // 时间点列表
        std::vector<Object> Objects_;           // 打击物件列表
        std::vector<std::string> Unknown_;      // 未能识别的原始行
        std::string version;                    // 文件头
        std::vector<std::string> order_;        // 各键值章节中条目的出现顺序

    public:
        Beatmap() = default;
        Beatmap(const Beatmap &) = default;
        Beatmap(Beatmap &&) = default;
        Beatmap &operator=(const Beatmap &) = default;
        Beatmap &operator=(Beatmap &&) = default;
        ~Beatmap() = default;

        // ======================== Getter ========================
        const std::map<std::string, std::string> &GetGeneral() const { return General_; }
        const std::map<std::string, std::string> &GetEditor() const { return Editor_; }
        const std::map<std::string, std::string> &GetMetadata() const { return Metadata_; }
        const std::map<std::string, std::string> &GetDifficulty() const { return Difficulty_; }
        const std::map<std::string, std::string> &GetColors() const { return Colors_; }
        /// @brief 返回各键值章节中条目的出现顺序
        const std::vector<std::string> &GetOrder() const { return order_; }
        const std::vector<Event> &GetEvents() const { return Events_; }
        const std::vector<TimingPoint> &GetTimingPoints() const { return TimingPoints_; }
        const std::vector<Object> &GetObjects() const { return Objects_; }
        /// @brief 返回未被识别、原样保留的行
        const std::vector<std::string> &GetUnknownLines() const { return Unknown_; }
        /// @brief 返回谱面文件版本
        std::string GetVersion() const { return version; }

        // ======================== Setter ========================
        void SetGeneral(const std::map<std::string, std::string> &general) { General_ = general; }
        void SetEditor(const std::map<std::string, std::string> &editor) { Editor_ = editor; }
        void SetMetadata(const std::map<std::string, std::string> &metadata) { Metadata_ = metadata; }
        void SetDifficulty(const std::map<std::string, std::string> &difficulty) { Difficulty_ = difficulty; }
        void SetColors(const std::map<std::string, std::string> &colors) { Colors_ = colors; }
        void SetEvents(const std::vector<Event> &events) { Events_ = events; }
        void SetTimingPoints(const std::vector<TimingPoint> &tps) { TimingPoints_ = tps; }
        void SetObjects(const std::vector<Object> &objects) { Objects_ = objects; }
        void SetOrder(const std::vector<std::string> &order) { order_ = order; }
        void SetUnknownLines(const std::vector<std::string> &unknown) { Unknown_ = unknown; }
        void SetVersion(const std::string &ver) { version = ver; }

        // 移动版本
        void SetGeneral(std::map<std::string, std::string> &&general) { General_ = std::move(general); }
        void SetEditor(std::map<std::string, std::string> &&editor) { Editor_ = std::move(editor); }
        void SetMetadata(std::map<std::string, std::string> &&metadata) { Metadata_ = std::move(metadata); }
        void SetDifficulty(std::map<std::string, std::string> &&difficulty) { Difficulty_ = std::move(difficulty); }
        void SetColors(std::map<std::string, std::string> &&colors) { Colors_ = std::move(colors); }
        void SetEvents(std::vector<Event> &&events) { Events_ = std::move(events); }
        void SetTimingPoints(std::vector<TimingPoint> &&tps) { TimingPoints_ = std::move(tps); }
        void SetObjects(std::vector<Object> &&objects) { Objects_ = std::move(objects); }
        void SetOrder(std::vector<std::string> &&order) { order_ = std::move(order); }
        void SetUnknownLines(std::vector<std::string> &&unknown) { Unknown_ = std::move(unknown); }
    };
}
