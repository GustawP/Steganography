cmake_minimum_required(VERSION 3.26)
project(Steganography)

set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS FALSE)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 11.0.2
)
FetchContent_Declare(
        SFML
        GIT_REPOSITORY  https://github.com/SFML/SFML
        GIT_TAG         2.6.1
)
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(SFML)

add_executable(Steganography main.cpp)
target_link_libraries(Steganography fmt)
target_link_libraries(Steganography sfml-graphics)
