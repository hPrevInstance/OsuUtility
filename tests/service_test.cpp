/**
 * @file    service_test.cpp
 * @brief   服务层原生单测（用 mock 进程执行器，无需真实 ffmpeg/ffprobe）
 *
 * 运行方式（项目根目录）：
 *   ./test.sh                       # 一键编译并运行全部测试
 * 或：
 *   cmake -S tests -B tests/build && cmake --build tests/build \
 *     && ctest --test-dir tests/build --output-on-failure
 */

#include "adapters/IProcessRunner.hpp"
#include "core/Fs.hpp"
#include "core/speed/AudioProcess.hpp"
#include "services/SpeedService.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
    class MockRunner : public adapters::IProcessRunner
    {
    public:
        adapters::ProcessResult Run(const std::string &program, const std::vector<std::string> &) override
        {
            if (program.find("ffprobe") != std::string::npos)
            {
                return adapters::ProcessResult{0, "44100\n", ""};
            }
            // ffmpeg：直接成功
            return adapters::ProcessResult{0, "", ""};
        }
    };
} // namespace

int main()
{
    auto tmp = core::fs::temp_directory_path() / "outil_service_test";
    core::fs::create_directories(tmp);

    const std::string src =
        "osu file format v14\n"
        "\n"
        "[General]\n"
        "AudioFilename: audio.mp3\n"
        "\n"
        "[Metadata]\n"
        "Title:Test\n"
        "Version:Easy\n"
        "\n"
        "[Events]\n"
        "0,0,bg.jpg,0,0\n"
        "\n"
        "[TimingPoints]\n"
        "500,400,4,2,1,60,1,0\n"
        "\n"
        "[HitObjects]\n"
        "100,100,1000,1,0,0:0:0:0:\n";

    auto in = tmp / "song.osu";
    {
        std::ofstream ofs(in);
        ofs << src;
    }
    {
        std::ofstream ofs(tmp / "bg.jpg");
        ofs << "dummy";
    }

    MockRunner runner;
    services::SpeedService service(runner);
    services::SpeedRequest req;
    req.beatmapPath = in;
    req.outputDir = tmp / "out";
    req.tempo = "2";
    req.mode = core::speed::TEMPO;

    auto out = service.Process(req);

    assert(core::fs::exists(out));
    assert(core::fs::exists(req.outputDir / "bg.jpg"));

    std::ifstream ifs(out);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    assert(content.find("Version:Easy (2)") != std::string::npos);
    assert(content.find("100,100,500,1,0") != std::string::npos); // 物件时间 1000 -> 500

    core::fs::remove_all(tmp);
    std::cout << "SERVICE TEST PASSED\n";
    return 0;
}
