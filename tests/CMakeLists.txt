find_package(Catch2 3 REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/tests/cmake")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(tests test_concurrent_hash_map.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
