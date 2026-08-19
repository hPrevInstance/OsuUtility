#!/usr/bin/env bash
# 一键编译并运行全部单元测试（原生编译，不交叉、不依赖 Qt）
set -e
cd "$(dirname "$0")"

cmake -S tests -B tests/build
cmake --build tests/build -j
ctest --test-dir tests/build --output-on-failure
