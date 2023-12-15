#include <abyss/concurrent_hash_map.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic map insert and find", "[concurrent_hash_map]") {
    abyss::ConcurrentHashMap<int, int> map(16);

    map.insert(1, 2);
    REQUIRE(map.find(1) == 2);
    REQUIRE(map.find(2) == 0); // Not found.

    map.insert(1, 3);
    map.insert(2, 4);
    map.insert(3, 5);
    REQUIRE(map.find(1) == 3);
    REQUIRE(map.find(2) == 4);
    REQUIRE(map.find(3) == 5);

    map.insert(1, 6);
    map.insert(2, 7);
    map.insert(3, 8);
    REQUIRE(map.find(1) == 6);
    REQUIRE(map.find(2) == 7);
    REQUIRE(map.find(3) == 8);
}
