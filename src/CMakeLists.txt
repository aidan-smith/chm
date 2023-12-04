

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/src/cmake")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(run main.cpp)
target_compile_options(run PRIVATE -g -Wall -Werror)
