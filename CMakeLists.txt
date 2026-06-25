cmake_minimum_required(VERSION 3.20)

project(kctboy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

add_executable(kctboy
    src/main.cpp
    src/GB/GB.cpp
    src/CPU/CPU.cpp
    src/CPU/ops/CPU_0_ops.cpp
    src/CPU/ops/CPU_1_ops.cpp
    src/CPU/ops/CPU_2_ops.cpp
    src/CPU/ops/CPU_3_ops.cpp
    src/CPU/ops/CPU_cb_ops.cpp
    src/MMU/MMU.cpp
    src/PPU/PPU.cpp
    src/APU/APU.cpp
)

target_include_directories(kctboy PRIVATE
    include/APU
    include/CPU
    include/GB
    include/misc
    include/MMU
    include/PPU
)

target_link_libraries(kctboy PRIVATE SDL3::SDL3)
