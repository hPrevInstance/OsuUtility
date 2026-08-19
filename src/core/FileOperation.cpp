/**
 * @file    FileOperation.cpp
 * @brief   谱面文件导出模块实现
 * @ingroup core
 */

#include "core/FileOperation.hpp"
#include "core/Error.hpp"

#include <fstream>
#include <string>
#include <vector>

namespace core
{
    void ExportFile(const fs::path &newdir, const fs::path &name, const std::vector<std::string> &content)
    {
        fs::create_directories(newdir);
        auto outPath = newdir / name.filename();
        std::ofstream ofs(outPath);
        if (!ofs)
        {
            throw MakeFileError("无法创建输出文件", outPath.string());
        }
        for (auto &line : content)
        {
            ofs << line << std::endl;
        }
        if (!ofs)
        {
            throw MakeFileError("写入输出文件失败", outPath.string());
        }
    }
}

