#pragma once

#include "ProcessToolsBase.hpp"
#include <QByteArray>
#include <QProcess>
namespace tools
{
    class BeatmapTask : public BeatmapTaskBase
    {
        static bool parseFFprobeOutput(const QByteArray &json, int &sampleRate, double &duration);

    public:
        using BeatmapTaskBase::BeatmapTaskBase;
        bool FetchAudioInfo(std::string *errorMsg = nullptr);
        bool ProcessAudioSync(const core::fs::path &outputPath, std::string *errorMsg = nullptr) const;
        void ProcessAudioAsync(const core::fs::path &outputPath,
                               std::function<void(bool, const std::string &)> callback) const;
    };
}