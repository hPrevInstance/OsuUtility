cmake_minimum_required(VERSION 3.16)

# 1. 设置项目名称和版本
project(SpeedConverter VERSION 1.0.0)

# 2. 指定交叉编译器（MinGW，生成 Windows exe）
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)

# 3. 开启 C++17 标准（现代 C++ 必备）
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. 关键：告诉 CMake 去哪里找 Windows 版的 Qt 库（注意改成你自己的实际路径）
set(CMAKE_PREFIX_PATH "/mnt/d/Qt/6.11.1/mingw_64") # 请核对版本号！

# 5. 寻找 Qt6 核心库和 Widgets 库（用于界面）
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 6. 将 src 目录下的所有 .cpp 文件收集为源文件变量（不包括 main.cpp）
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
# 将 include 目录添加到头文件搜索路径
include_directories(include)

# 7. 生成可执行文件（将 main.cpp 和 src 下的逻辑合并）
add_executable(${PROJECT_NAME} 
    app/main.cpp
    ${SRC_FILES}
    include/core/TextProcessor.h
    src/core/AudioProcessor.cpp
)

# 8. 链接 Qt 库
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets
)

target_link_options(${PROJECT_NAME} PRIVATE -mwindows)