/**
 * @file    SpeedCommand.hpp
 * @brief   speed 子命令
 *
 * 提供 osu 谱面与音频批量变速/变调的 CLI 子命令。
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup cli
 */

#pragma once

#include "CLI11.hpp"
#include "Command.hpp"
#include "core/Fs.hpp"

#include <string>
#include <vector>

namespace cli
{
    /**
     * @brief 每谱面不同速度的条目
     */
    struct DiffEntry
    {
        std::string tempo; // 变速
        std::string pitch; // 变调
    };

    /**
     * @brief 校验速度字符串，返回空串表示合法，否则返回错误信息
     */
    std::string ValidateSpeed(const std::string &input);

    /**
     * @brief 供 CLI11 使用的速度校验器
     */
    inline CLI::Validator SpeedValidator()
    {
        return CLI::Validator(
            [](std::string &value) -> std::string
            { return ValidateSpeed(value); },
            "倍速，小数或分数，如 1.5、3/2",
            "SPEED");
    }

    /**
     * @brief speed 子命令：批量调整 .osu 谱面与音频的变速/变调
     */
    class SpeedCommand : public Command
    {
    public:
        void Register(CLI::App &app) override;
        bool Selected() const override { return sub_ != nullptr && sub_->parsed(); }
        int Run() override;

    private:
        CLI::App *sub_ = nullptr;     // 本命令对应的子命令对象
        CLI::Option *optDiff_ = nullptr; // 差异模式选项，用于判断是否被指定

        //  选项状态
        std::vector<core::fs::path> files_; // 输入文件/目录
        core::fs::path output_;             // 输出目录
        bool recursive_ = false;            // 是否递归遍历

        std::string speed_;  // 同时变速变调
        std::string tempo_;  // 变速
        std::string pitch_;  // 变调
        std::string diffFile_; // 差异模式映射文件

        bool dryRun_ = false;     // 试运行
        bool quiet_ = false;      // 静默
        bool verbose_ = false;    // 详细输出
        bool forceColor_ = false; // 强制彩色
    };
} // namespace cli
