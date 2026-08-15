cmake_minimum_required(VERSION 3.16)

project(OsuUtility VERSION 1.0.0)

#  1. 编译器与标准 
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#  2. Qt 环境 
set(CMAKE_PREFIX_PATH "/mnt/d/Qt/6.11.1/mingw_64")
set(QT_NO_DEPLOYMENT_TOOL TRUE)
set(QT_DEPLOY_SUPPORT OFF) 
set(CMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_Vulkan TRUE)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

#  3. 自动处理 Qt 的 Q_OBJECT 宏 
set(CMAKE_AUTOMOC ON)   # 自动运行 moc，生成元对象代码

#  4. 头文件搜索路径 
include_directories(include)   # 所有 #include 都从 include/ 开始搜索

#  5. 主要可执行文件 
add_executable(${PROJECT_NAME}
    app/main.cpp                   # 程序入口
    src/cli/CLIHandler.cpp         # CLI 顶层分发
    src/cli/SpeedCommand.cpp       # speed 子命令
    src/tools/BeatmapTaskBase.cpp  # 谱面处理同步基类实现
    src/core/AudioProcess.cpp      # 音频倍速参数处理实现
    src/core/FileOperation.cpp     # 谱面文件解析与倍速调整实现
    src/core/Difficulty.cpp        # osu 谱面文件解析类实现
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#  6. 编译定义（宏） 
target_compile_definitions(${PROJECT_NAME} PRIVATE
    FMT_HEADER_ONLY             # fmt 库要求
)

#  7. 编译选项（按构建类型区分） 
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3 -ffunction-sections -fdata-sections>
    $<$<CONFIG:Debug>:-g -O0>
    -finput-charset=UTF-8
    -fexec-charset=UTF-8
)

#  8. 链接选项 
target_link_options(${PROJECT_NAME} PRIVATE
    "LINKER:--gc-sections"      # 减少无用代码，优化体积
)

#  9. 链接 Qt 库 
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
)

#  10. 输出文件名 
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    OUTPUT_NAME "outil"           # 生成 outil.exe 
    DEBUG_OUTPUT_NAME "outil_d"   # Debug 模式下生成 outil_d.exe
)

#  11. (可选) 隐藏控制台窗口 
# 如果将来需要纯 GUI 模式，取消下面注释（但会丢失 std::cout 输出）
# target_link_options(${PROJECT_NAME} PRIVATE -mwindows)

#  12. (可选) 设置默认构建类型 
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()