cmake_minimum_required(VERSION 3.10)
project(BezierCurve)

set(CMAKE_CXX_STANDARD 17)

# 必须写在 find_package 前面
# 这里必须是包含 OpenCVConfig.cmake 的文件夹
set(OpenCV_DIR
        "D:/games101/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/x64/mingw/lib"
)

find_package(OpenCV REQUIRED)

add_executable(BezierCurve main.cpp)

target_include_directories(BezierCurve PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(BezierCurve PRIVATE
        ${OpenCV_LIBS}
)