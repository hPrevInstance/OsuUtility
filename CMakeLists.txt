cmake_minimum_required(VERSION 3.16)

project(SpeedConverter VERSION 1.0.0)
set(QT_NO_DEPLOYMENT_TOOL TRUE) 

set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "/mnt/d/Qt/6.11.1/mingw_64") 

set(CMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_Vulkan TRUE)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

include_directories(include)

add_executable(${PROJECT_NAME} 
    app/main.cpp
    include/core/FileOperation.h
    include/core/AudioProcess.h
    include/gui/MainWindow.h
)

target_compile_options(${PROJECT_NAME} 
    PRIVATE
    -O3
    -ffunction-sections
    -fdata-sections)

target_link_options(${PROJECT_NAME}
    PRIVATE
    "LINKER:--gc-sections"
    )

target_compile_definitions(${PROJECT_NAME} PRIVATE FMT_HEADER_ONLY)

target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets
)

#target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "spdconv")

# 添加调试信息，并关闭优化，方便调试
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_BUILD_TYPE Debug)