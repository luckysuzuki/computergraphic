cmake_minimum_required(VERSION 3.21)
project(ComputerGraphic LANGUAGES CXX)

# 从 cg 根目录直接打开 CLion，默认提供当前正在学习的作业 7。
# 其他作业继续保留独立项目，避免引入未配置的 OpenCV / OpenGL 依赖。
include(CTest)
add_subdirectory(GAMES101_Homework7_S2021/Assignment7)
