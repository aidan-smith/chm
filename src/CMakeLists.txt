find_package(Boost REQUIRED)
find_package(Folly REQUIRED)
# find_package(TBB REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/src/cmake")

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIRS})
# include_directories(${FOLLY_INCLUDE_DIR})


add_executable(run main.cpp)
target_compile_options(run PRIVATE -g -Wall -Werror)
target_link_libraries(run ${Boost_LIBRARIES})
