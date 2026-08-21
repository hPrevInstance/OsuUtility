cmake_minimum_required(VERSION 3.16)
project(OsuUtilityTests LANGUAGES CXX)

# 独立于主工程的测试工程：使用【原生】编译器，不交叉编译、不依赖 Qt。
# core 与 services 层均为纯 C++（无 Qt、无平台特定代码），可原生编译运行。
#
# 运行方式（在项目根目录）：
#   ./test.sh                         # 一键配置 + 编译 + 运行
# 或手动：
#   cmake -S tests -B tests/build
#   cmake --build tests/build -j
#   ctest --test-dir tests/build --output-on-failure

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# 项目根目录（include 头文件与 core/services 源码所在）
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# core 纯算法静态库（无 Qt、无外部进程）
add_library(outil_core STATIC
    ${PROJECT_ROOT}/src/core/beatmap/BeatmapParser.cpp
    ${PROJECT_ROOT}/src/core/beatmap/BeatmapSerializer.cpp
    ${PROJECT_ROOT}/src/core/speed/AudioProcess.cpp
    ${PROJECT_ROOT}/src/core/speed/SpeedTransform.cpp
    ${PROJECT_ROOT}/src/core/FileOperation.cpp
)
target_include_directories(outil_core PUBLIC ${PROJECT_ROOT}/include)

# 核心算法测试
add_executable(core_test core_test.cpp)
target_link_libraries(core_test PRIVATE outil_core)

# 服务层测试（SpeedService 只依赖 IProcessRunner 接口 + core，无 Qt）
add_executable(service_test
    service_test.cpp
    ${PROJECT_ROOT}/src/services/SpeedService.cpp
)
target_link_libraries(service_test PRIVATE outil_core)

add_test(NAME core_test COMMAND core_test)
add_test(NAME service_test COMMAND service_test)
