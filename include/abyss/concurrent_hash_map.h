#ifndef CONCURRENT_HASH_MAP_H_
#define CONCURRENT_HASH_MAP_H_

#include <abyss/directives.h>

#include <atomic>
#include <cstddef>

namespace abyss {

// MurmurHash3
ABYSS_INLINE uint32_t fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

// MurmurHash3
ABYSS_INLINE uint64_t fmix64(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdLLU;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53LLU;
    h ^= h >> 33;

    if (h == 0) h = 0x7000000000000000LLU;
    if (h == 1) h = 0x7000000000000001LLU;
    if (h == 2) h = 0x7000000000000002LLU;

    return h;
}

template <typename K, typename V>
struct Entry {
    std::atomic<size_t> hash;
    std::atomic<V> value;
};

template <typename K, typename V>
struct Table {
    static constexpr size_t MIN_SIZE = 1 << 3;
    size_t size;
    std::atomic<ssize_t> slots_remaining;
    std::atomic<Table *> next;
    Entry<K, V> entries[0];

    static Table *init(size_t size, float max_load_factor) {
        Table *table = (Table *)malloc(sizeof(Table) + sizeof(Entry<K, V>) * size);
        table->size = size;
        table->slots_remaining = size * max_load_factor + 1;
        table->next = nullptr;
        return table;
    }

    void destroy() {
        this->~Table();
        free(this);
    }

    Entry<K, V> *get_cells() {
        return entries;
    }
};

// template <typename K, typename V>
// struct TableResizer {
//     Table<K, V> *old_table;
//     Table<K, V> *new_table;
//     size_t index;
// };

template <typename K, typename V>
class ConcurrentHashMap {
public:
    ConcurrentHashMap(size_t size = 0);
    ~ConcurrentHashMap();

    void clear(){};
    bool empty() const { return table_->size_ == 0; };
    size_t size() const { return table_->size; };
    size_t load() const { return table_->slots_remaining; };
    size_t hash(const K &key) const { return fmix64(std::hash<K>{}(key)); };

    void resize(size_t size);
    void commit_resize();

    V find(const K &key) const;
    void insert(const K &key, const V &value);
    void erase(const K &key);
    bool contains(const K &key) const;

    V &operator[](const K &key);
    const V &operator[](const K &key) const;

private:
    constexpr static V NULL_VAL = V();
    constexpr static size_t NULL_HASH = 0;
    constexpr static size_t MOVED_HASH = 1;
    constexpr static size_t TOMBSTONE_HASH = 2;
    Table<K, V> *table_;
    float max_load_factor_;
};

};  // namespace abyss

#include "../../src/concurrent_hash_map.cpp"

#endif  // CONCURRENT_HASH_MAP_H_
