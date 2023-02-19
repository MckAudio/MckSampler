cmake_minimum_required(VERSION 3.20)

project(MckSampler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(JACK NAMES jack PATHS /usr/lib /usr/lib64 /usr/lib64/pipewire-0.3/jack)

message(STATUS "Found jack:\t\t${JACK}")

add_subdirectory(src/helper)
add_subdirectory(src/gui)
add_subdirectory(src/q/q_lib)
add_subdirectory(src/q/infra)

add_executable(mck-sampler
    src/main.cpp
    src/Config.cpp
    src/ConfigFile.cpp
    src/Processing.cpp
    src/SampleExplorer.cpp
    src/Types.cpp
    src/types/cpp/Sampler.cpp
    src/types/cpp/Samples.cpp
    src/types/cpp/System.cpp
    src/types/cpp/Transport.cpp
)

target_compile_definitions(mck-sampler PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_include_directories(mck-sampler
    PRIVATE
    src/gui/include
    src/gui/concurrentqueue
    src/helper/include
    src/types/hpp
)

target_link_libraries(mck-sampler
    PRIVATE
    MckHelper
    MckGui
    ${JACK}
    cycfi::infra
    libq
)