#include <abyss/concurrent_hash_map.h>
#include <abyss/directives.h>
#include <math.h>

#include <boost/unordered/concurrent_flat_map.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <latch>
#include <random>
#include <thread>
#include <shared_mutex>

struct std_map {
    std::unordered_map<int, int> map;
    boost::shared_mutex mut;

    std_map(size_t size) {
        map.reserve(size);
    }

    bool contains(int key) {
        boost::shared_lock<boost::shared_mutex> lock(mut);
        return map.contains(key);
    }

    void insert(int key, int value) {
        boost::unique_lock<boost::shared_mutex> lock(mut);
        map.insert({key, value});
    }
};

using my_map = abyss::ConcurrentHashMap<int, int>;
using boost_map = boost::concurrent_flat_map<int, int>;
// using folly_map = folly::ConcurrentHashMap<int, int>;

template <typename K>
ABYSS_INLINE void map_find(const my_map& map, const K& key) {
    map.contains(key);
}

template <typename... Args>
ABYSS_INLINE void map_update(my_map& map, Args&&... args) {
    map.insert(std::forward<Args>(args)...);
}

template <typename K>
ABYSS_INLINE void map_find(std_map& map, const K& key) {
    map.contains(key);
}

template <typename... Args>
ABYSS_INLINE void map_update(std_map& map, Args&&... args) {
    map.insert(std::forward<Args>(args)...);
}

template <typename K>
ABYSS_INLINE void map_find(const boost_map& map, const K& key) {
    map.contains(key);
}

template <typename... Args>
ABYSS_INLINE void map_update(boost_map& map, Args&&... args) {
    map.emplace_or_visit(std::forward<Args>(args)..., [](auto& x) { ++x.second; });
}

// template <typename K>
// ABYSS_INLINE void map_find(const folly_map& map, const K& key) {
//     map.find(key);
// }
//
// template <typename... Args>
// ABYSS_INLINE void map_update(folly_map& map, Args&&... args) {
//     map.insert(std::forward<Args>(args)...);
// }

// run test on N threads
template <typename Map>
void run_test(size_t num_threads, size_t working_set = 100'000, size_t ops_per_thread = 10'000'000) {
    using std::chrono::high_resolution_clock;
    std::cout << "Running test on " << num_threads << " threads:\n";

    Map map(working_set);
    std::vector<std::thread> threads;
    std::latch ready(num_threads), start(1), completed(num_threads), finish(1);

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Init stage
            (void)i;
            std::random_device r;
            std::mt19937 e(r());
            std::discrete_distribution<> action{75, 25}; // Read vs Write bias
            std::uniform_int_distribution<int> element(1, working_set);

            // Wait for all threads to be ready
            ready.count_down();
            start.wait();

            // Run test
            for (size_t i = 0; i < ops_per_thread; ++i) {
                int key = element(e);

                switch (action(e)) {
                    case 0:
                        map_find(map, key);
                        break;
                    case 1:
                        map_update(map, key | 1L, 0); // Aim for 0.5 load factor.
                        break;
                    default:
                        std::cerr << "Invalid action\n";
                        exit(1);
                }
            }
            completed.count_down();
            finish.wait();
        });
    }

    ready.wait();
    auto t1 = high_resolution_clock::now();
    start.count_down();
    completed.wait();
    auto t2 = high_resolution_clock::now();
    finish.count_down();

    for (auto& t : threads) {
        t.join();
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto throughput = (ops_per_thread * num_threads) / (elapsed / 1000.0) / 1'000'000.0;
    std::cout << "    Time: " << elapsed << "ms\n"
              << "    Throughput: " << throughput << "Mops/s\n";

}

int main() {
    using std::chrono::high_resolution_clock;
    std::cout << "Starting Benchmark!\n";

    int N = 16;

    std::cout << "Running tests on std_map:\n";
    for (int n = 1; n <= N; n++) {
        run_test<std_map>(n, 20'000, 50'000);
    }

    std::cout << "Running tests on my_map:\n";
    for (int n = 1; n <= N; n++) {
        run_test<my_map>(n, 200'000);
    }

    std::cout << "Running tests on boost_map:\n";
    for (int n = 1; n <= N; n++) {
        run_test<boost_map>(n, 200'000);
    }

    std::cout << "Ending Benchmark!\n";
    return 0;
}
