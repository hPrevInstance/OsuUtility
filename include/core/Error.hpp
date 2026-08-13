/**
 * @file    Error.hpp
 * @brief   统一异常与退出码定义
 *
 * 为整个项目提供一致的异常类型与退出码，便于 CLI / GUI 统一处理：
 *  - OspError      : 统一异常类型，携带错误分类
 *  - 工厂函数      : MakeParamError / MakeFileError / MakeProcessError / MakeInternalError
 *  - 退出码常量    : EXIT_OK / EXIT_PARTIAL_FAILURE / EXIT_USAGE / EXIT_EXTERNAL
 *
 * @author  hPrevInstance
 * @version 1.0.0
 * @ingroup core
 */

#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace core
{
    /**
     * @brief 错误分类
     */
    enum class ErrorKind
    {
        Parameter,       ///< 参数 / 速度错误
        File,            ///< 文件读写错误
        ExternalProcess, ///< 外部工具（ffmpeg / ffprobe）错误
        Internal,        ///< 内部逻辑错误
    };

    /**
     * @brief 统一的进程退出码
     */
    inline constexpr int EXIT_OK = 0;              ///< 全部成功
    inline constexpr int EXIT_PARTIAL_FAILURE = 1; ///< 部分文件处理失败
    inline constexpr int EXIT_USAGE = 2;           ///< 参数 / 用法错误
    inline constexpr int EXIT_EXTERNAL = 3;        ///< 外部工具缺失 / 失败

    /**
     * @brief 返回错误分类的中文名称
     */
    inline const char *ErrorKindName(ErrorKind kind) noexcept
    {
        switch (kind)
        {
        case ErrorKind::Parameter:
            return "参数错误";
        case ErrorKind::File:
            return "文件错误";
        case ErrorKind::ExternalProcess:
            return "外部进程错误";
        case ErrorKind::Internal:
            return "内部错误";
        }
        return "未知错误";
    }

    /**
     * @brief 项目统一异常类型
     *
     * 继承自 std::runtime_error，额外携带错误分类，
     * what() 输出统一的人性化中文消息。
     */
    class OspError : public std::runtime_error
    {
    private:
        ErrorKind kind_;

        static std::string BuildMessage(ErrorKind kind, const std::string &msg)
        {
            return std::string(ErrorKindName(kind)) + "：" + msg;
        }

    public:
        OspError(ErrorKind kind, std::string message)
            : std::runtime_error(BuildMessage(kind, message)), kind_(kind)
        {
        }

        /// @brief 返回错误分类
        ErrorKind Kind() const noexcept { return kind_; }
    };

    /**
     * @name 异常工厂函数
     * @brief 便于在 throw 语句中直接构造带分类的异常
     */
    ///@{
    inline OspError MakeParamError(const std::string &msg)
    {
        return OspError(ErrorKind::Parameter, msg);
    }

    inline OspError MakeFileError(const std::string &msg, const std::string &file = std::string())
    {
        return OspError(ErrorKind::File, file.empty() ? msg : msg + "（文件：" + file + "）");
    }

    inline OspError MakeProcessError(const std::string &msg, const std::string &file = std::string())
    {
        return OspError(ErrorKind::ExternalProcess, file.empty() ? msg : msg + "（文件：" + file + "）");
    }

    inline OspError MakeInternalError(const std::string &msg, const std::string &file = std::string())
    {
        return OspError(ErrorKind::Internal, file.empty() ? msg : msg + "（文件：" + file + "）");
    }
    ///@}
}
