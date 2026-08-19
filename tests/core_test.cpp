/**
 * @file    core_test.cpp
 * @brief   核心模块原生单测（无需 Qt / 无需交叉编译）
 *
 * 验证 core/beatmap 与 core/speed 纯算法的正确性：
 *  - ParseFraction / Factoring / GenerateFilterChain
 *  - BeatmapParser 解析
 *  - SpeedTransform 倍速变换
 *  - BeatmapSerializer 序列化
 *
 * 运行方式（项目根目录）：
 *   ./test.sh                       # 一键编译并运行全部测试
 * 或：
 *   cmake -S tests -B tests/build && cmake --build tests/build \
 *     && ctest --test-dir tests/build --output-on-failure
 */

#include "core/beatmap/BeatmapParser.hpp"
#include "core/beatmap/BeatmapSerializer.hpp"
#include "core/speed/AudioProcess.hpp"
#include "core/speed/SpeedTransform.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

int main()
{
    // 1. 分数解析
    {
        auto [n, d] = core::speed::ParseFraction("1.5");
        assert(n == 3 && d == 2);
        auto [n2, d2] = core::speed::ParseFraction("3/2");
        assert(n2 == 3 && d2 == 2);
        auto [n3, d3] = core::speed::ParseFraction("2");
        assert(n3 == 2 && d3 == 1);
    }

    // 2. 因子分解
    {
        auto f = core::speed::Factoring({3, 2});
        assert(!f.empty());
        // 3/2 -> [2, 3/4]，每个因子应在 [1/2, 2] 内
        for (const auto &fact : f)
        {
            std::cout << "factor: " << fact << "\n";
        }
    }

    // 3. 滤镜链（变速不变调）
    {
        auto chain = core::speed::GenerateFilterChain("1.5", "", 44100, core::speed::TEMPO);
        assert(chain.find("atempo") != std::string::npos);
        std::cout << "tempo chain: " << chain << "\n";
    }

    // 4. 解析 -> 变换 -> 序列化
    std::string src =
        "osu file format v14\n"
        "\n"
        "[General]\n"
        "AudioFilename: audio.mp3\n"
        "PreviewTime: 10000\n"
        "\n"
        "[Metadata]\n"
        "Title:Test\n"
        "Version:Easy\n"
        "BeatmapID:123\n"
        "BeatmapSetID:456\n"
        "\n"
        "[Difficulty]\n"
        "HPDrainRate:5\n"
        "\n"
        "[Events]\n"
        "0,0,bg.jpg,0,0\n"
        "2,10000,20000\n"
        "\n"
        "[TimingPoints]\n"
        "500,400,4,2,1,60,1,0\n"
        "\n"
        "[HitObjects]\n"
        "100,100,1000,1,0,0:0:0:0:\n";

    std::istringstream iss(src);
    auto bmp = core::BeatmapParser::Parse(iss);

    assert(bmp.GetMetadata().at("Title") == "Test");
    assert(bmp.GetObjects().size() == 1);
    assert(bmp.GetObjects().at(0).time == 1000);
    assert(bmp.GetTimingPoints().size() == 1);
    assert(bmp.GetEvents().size() == 2);

    core::speed::MediaInfo media;
    auto out = core::speed::Transform(bmp, 2.0, media);

    assert(media.audio == "audio.mp3");
    assert(media.background == "bg.jpg");
    assert(out.GetGeneral().at("PreviewTime") == "5000");
    assert(out.GetMetadata().at("Version").find("(2)") != std::string::npos);
    assert(out.GetMetadata().at("BeatmapID") == "0");
    assert(out.GetObjects().at(0).time == 500);
    assert(out.GetTimingPoints().at(0).time == 250.0);
    assert(out.GetTimingPoints().at(0).beat_length == 200.0);
    // 事件开始/结束时间
    assert(out.GetEvents().at(1).start_time == 5000);
    assert(out.GetEvents().at(1).args.at(0) == "10000");

    auto lines = core::BeatmapSerializer::ToLines(out);
    std::cout << "--- serialized ---\n";
    for (const auto &l : lines)
    {
        std::cout << l << "\n";
    }

    std::cout << "ALL CORE TESTS PASSED\n";
    return 0;
}
