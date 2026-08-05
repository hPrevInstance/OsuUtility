#pragma once

#include "ProcessToolsBase.hpp"
#include <QByteArray>
#include <QProcess>
#include <QString>

namespace tools
{
    class BeatmapTask : public BeatmapTaskBase
    {
        SongInfo si_;
        bool FetchAudioInfo(std::string *errorMsg = nullptr) override;
        bool ProcessAudio(const core::fs::path &outputPath, std::string *errorMsg = nullptr) const override;
        bool ParseFFprobeOutput(const QString json);

    public:
        using BeatmapTaskBase::BeatmapTaskBase;
        void Parse(const core::fs::path &newdir) override;
    };
}