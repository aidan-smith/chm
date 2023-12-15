#include <abyss/concurrent_hash_map.h>
#include <abyss/directives.h>
#include <math.h>

#include <boost/unordered/concurrent_flat_map.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <latch>
#include <random>
#include <thread>

using my_map = abyss::ConcurrentHashMap<int, int>;
using boost_map = boost::concurrent_flat_map<int, int>;

template <typename K>
ABYSS_INLINE void map_find(const my_map& map, const K& key) {
    map.contains(key);
}

template <typename... Args>
ABYSS_INLINE void map_update(my_map& map, Args&&... args) {
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

// run test on N threads
template <typename Map>
void run_test(size_t num_threads, size_t working_set = 100'000) {
    using std::chrono::high_resolution_clock;
    std::cout << "Running test on " << num_threads << " threads:\n";

    size_t ops_per_thread = 10'000'000;
    Map map(20'000'000);
    std::vector<std::thread> threads;
    std::latch ready(num_threads), start(1), completed(num_threads), finish(1);

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Init stage
            (void)i;
            std::random_device r;
            std::mt19937 e(r());
            std::discrete_distribution<> action{90, 10};
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
                        map_update(map, key | 1L, 0);
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

    std::cout << "    Time: " << elapsed << "ms\n"
              << "    Throughput: "
              << (ops_per_thread * num_threads) / (elapsed / 1000.0) / 1'000'000.0 << "Mops/s\n"
              << "    Size: " << map.size() << "\n";
}

int main() {
    using std::chrono::high_resolution_clock;
    std::cout << "Starting Benchmark!\n";

    int N = 16;

    for (int n = 1; n <= N; n <<= 1) {
        run_test<boost_map>(n);
        run_test<my_map>(n);
    }

    std::cout << "Ending Benchmark!\n";
    return 0;
}
