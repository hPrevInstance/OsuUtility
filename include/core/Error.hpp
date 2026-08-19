/**
 * @file    Error.hpp
 * @brief   统一异常定义
 *
 * 为整个项目提供一致的异常类型，便于服务层与 GUI 统一处理：
 *  - OutilError    : 统一异常类型，携带错误分类
 *  - 工厂函数      : MakeParamError / MakeFileError / MakeProcessError / MakeInternalError
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
        Parameter,       // 参数 / 速度错误
        File,            // 文件读写错误
        ExternalProcess, // 外部工具错误
        Internal,        // 内部逻辑错误
    };

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
    class OutilError : public std::runtime_error
    {
    private:
        ErrorKind kind_;

        static std::string BuildMessage(ErrorKind kind, const std::string &msg)
        {
            return std::string(ErrorKindName(kind)) + "：" + msg;
        }

    public:
        OutilError(ErrorKind kind, std::string message)
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
    inline OutilError MakeParamError(const std::string &msg)
    {
        return OutilError(ErrorKind::Parameter, msg);
    }

    inline OutilError MakeFileError(const std::string &msg, const std::string &file = std::string())
    {
        return OutilError(ErrorKind::File, file.empty() ? msg : msg + "（文件：" + file + "）");
    }

    inline OutilError MakeProcessError(const std::string &msg, const std::string &file = std::string())
    {
        return OutilError(ErrorKind::ExternalProcess, file.empty() ? msg : msg + "（文件：" + file + "）");
    }

    inline OutilError MakeInternalError(const std::string &msg, const std::string &file = std::string())
    {
        return OutilError(ErrorKind::Internal, file.empty() ? msg : msg + "（文件：" + file + "）");
    }
    ///@}
}
