cmake_minimum_required(VERSION 3.10)
project(RayTracing)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件列表（包含 .cpp 和 .hpp，但 .hpp 通常不需要显式列出，不过保留也无妨）
add_executable(RayTracing
        main.cpp
        Object.hpp
        Vector.hpp
        Sphere.hpp
        global.hpp
        Triangle.hpp
        Scene.cpp
        Scene.hpp
        Light.hpp
        Renderer.cpp
)

# 编译警告。MSVC 和 GCC/Clang 使用各自支持的参数。
if (MSVC)
    target_compile_options(RayTracing PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(RayTracing PRIVATE
            -Wall -Wextra -pedantic -Wshadow -Wreturn-type
    )
endif ()

# 显式启用 C++17 特性（其实上方已设置标准，这条可省略，但保留以匹配原风格）
target_compile_features(RayTracing PRIVATE cxx_std_17)

# MinGW 工具链通常不随附 libubsan；强制启用会导致链接失败。
# 在提供 UBSan 运行库的平台上仍保留原来的检查能力。
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MINGW)
    target_compile_options(RayTracing PRIVATE -fsanitize=undefined)
    target_link_libraries(RayTracing PRIVATE -fsanitize=undefined)
endif ()
