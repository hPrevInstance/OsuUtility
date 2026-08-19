/**
 * @file    QProcessRunner.cpp
 * @brief   QProcessRunner 实现
 * @ingroup adapters
 */

#include "adapters/QProcessRunner.hpp"

#include <QProcess>
#include <QStringList>

namespace adapters
{
    ProcessResult QProcessRunner::Run(const std::string &program, const std::vector<std::string> &args)
    {
        QProcess proc;

        QStringList qargs;
        qargs.reserve(static_cast<int>(args.size()));
        for (const auto &a : args)
        {
            qargs << QString::fromStdString(a);
        }

        proc.start(QString::fromStdString(program), qargs);
        if (!proc.waitForStarted(10000))
        {
            return ProcessResult{-1, "", "进程启动失败：" + program};
        }
        proc.closeWriteChannel();

        if (!proc.waitForFinished(-1))
        {
            proc.kill();
            proc.waitForFinished();
            return ProcessResult{-1, "", "进程执行超时或被终止：" + program};
        }

        ProcessResult r;
        r.exitCode = proc.exitCode();
        r.stdOut = proc.readAllStandardOutput().toStdString();
        r.stdErr = proc.readAllStandardError().toStdString();
        return r;
    }
}
