cmake_minimum_required(VERSION 3.25.1)

project(abyss)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

add_subdirectory(src)
add_subdirectory(tests)

