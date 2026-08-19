cmake_minimum_required(VERSION 3.16)

project(OsuUtility VERSION 1.0.0)

# ============================================================
#  构建说明
#  - WSL 下交叉编译到 Windows：
#      cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
#      （toolchain.cmake 已指定 MinGW 编译器与 Qt 前缀路径）
#  - Qt Creator（Windows）下：
#      直接用 Qt Creator 打开本目录的 CMakeLists.txt，选择配置好的 Qt Kit 即可
#      （Kit 会自动提供编译器与 CMAKE_PREFIX_PATH）。
#  编译器与 Qt 前缀【不要】写死在 CMakeLists.txt 里，交由 toolchain / Qt Kit 决定。
# ============================================================

#  1. 语言标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#  2. Qt 环境（Qt 前缀由 toolchain 或 Qt Creator Kit 提供）
set(QT_NO_DEPLOYMENT_TOOL TRUE)
set(QT_DEPLOY_SUPPORT OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_Vulkan TRUE)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)

#  3. 自动运行 Qt 的元对象工具
set(CMAKE_AUTOMOC ON)   # moc：处理 Q_OBJECT 宏
set(CMAKE_AUTOUIC ON)   # uic：由 .ui 生成 ui_*.h（方便在 Qt Designer 中设计界面）
set(CMAKE_AUTORCC ON)   # rcc：编译 .qrc 资源（暂未使用，预留）

#  4. 头文件搜索路径
include_directories(include src)   # 头文件从 include/ 与 src/ 开始搜索

#  5. 主要可执行文件
add_executable(${PROJECT_NAME}
    app/main.cpp                       # 程序入口
    src/app/MainWindow.cpp             # 主窗口（Qt Widgets）
    src/app/MainWindow.ui              # 主窗口界面（Qt Designer）
    src/adapters/QProcessRunner.cpp    # 外部进程适配（QProcess 实现）
    src/services/SpeedService.cpp      # 倍速服务（编排 core + adapters）
    src/core/speed/AudioProcess.cpp    # 音频倍速参数处理实现
    src/core/speed/SpeedTransform.cpp  # 谱面倍速变换纯算法实现
    src/core/beatmap/BeatmapParser.cpp # osu 谱面文件解析实现
    src/core/beatmap/BeatmapSerializer.cpp # 谱面序列化实现
    src/core/FileOperation.cpp         # 谱面文件导出实现
)

#  6. 编译定义（宏）
target_compile_definitions(${PROJECT_NAME} PRIVATE
    FMT_HEADER_ONLY             # fmt 库要求
)

#  7. 编译器相关的优化/编码选项（仅 GCC/Clang，避免破坏 MSVC Kit）
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-O3 -ffunction-sections -fdata-sections>
        $<$<CONFIG:Debug>:-g -O0>
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        "LINKER:--gc-sections"      # 减少无用代码，优化体积
    )
endif()

#  8. 链接 Qt 库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
)

#  9. 输出文件名
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    OUTPUT_NAME "outil"           # 生成 outil.exe
    DEBUG_OUTPUT_NAME "outil_d"   # Debug 模式下生成 outil_d.exe
)

#  10. (可选) 隐藏控制台窗口
# 如果将来需要纯 GUI 模式，取消下面注释（但会丢失 std::cout 输出）
# target_link_options(${PROJECT_NAME} PRIVATE -mwindows)

#  11. (可选) 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
