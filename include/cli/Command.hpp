/**
 * @file    Command.hpp
 * @brief   CLI 子命令基类接口
 *
 * 定义功能子命令的统一接口，便于顶层 RunCLI 统一注册与分发。
 * 新增功能时派生本接口，实现 Register / Selected / Run 三个方法，
 * 并在 RunCLI 中注册即可。
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup cli
 */

#pragma once

#include "CLI11.hpp"

namespace cli
{
    /**
     * @brief CLI 子命令基类接口
     *
     *  - Register : 向顶层应用注册子命令并定义其选项
     *  - Selected : 该子命令是否被用户在本次运行中选中
     *  - Run      : 执行子命令的具体逻辑，返回进程退出码
     */
    class Command
    {
    public:
        virtual ~Command() = default;

        /**
         * @brief 注册子命令及其选项
         * @param app 顶层 CLI11 应用
         */
        virtual void Register(CLI::App &app) = 0;

        /**
         * @brief 判断本子命令是否被选中
         * @return true 已选中
         */
        virtual bool Selected() const = 0;

        /**
         * @brief 执行子命令
         * @return 进程退出码
         */
        virtual int Run() = 0;
    };
} // namespace cli
