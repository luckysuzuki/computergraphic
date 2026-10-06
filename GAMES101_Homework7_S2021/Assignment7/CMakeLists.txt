cmake_minimum_required(VERSION 3.10)
project(RayTracing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(RayTracing main.cpp Object.hpp Vector.cpp Vector.hpp Sphere.hpp global.hpp Triangle.hpp Scene.cpp
        Scene.hpp Light.hpp AreaLight.hpp BVH.cpp BVH.hpp Bounds3.hpp Ray.hpp Material.hpp Intersection.hpp
        Renderer.cpp Renderer.hpp)

# 使用源码目录中的模型，使运行时不依赖 IDE 的工作目录。
target_compile_definitions(RayTracing PRIVATE
        HW7_MODEL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/models/cornellbox")

# 源文件统一使用 UTF-8，避免中文注释受 Windows 默认编码影响。
if(MSVC)
    target_compile_options(RayTracing PRIVATE /utf-8)
else()
    target_compile_options(RayTracing PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 这里只检查环境和模型，不执行尚未完成的作业算法。
add_custom_target(check-scene
        COMMAND $<TARGET_FILE:RayTracing> --check-scene
        DEPENDS RayTracing
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# 几何边界和面积采样分布会直接影响光照正确性，提供独立回归检查。
include(CTest)
if(BUILD_TESTING)
    add_executable(HW7Tests tests.cpp BVH.cpp Scene.cpp Vector.cpp)
    add_test(NAME geometry-and-sampling COMMAND HW7Tests)
endif()
