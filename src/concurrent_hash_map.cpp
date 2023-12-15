#include <abyss/concurrent_hash_map.h>
#include <abyss/directives.h>

#include <algorithm>
#include <cstdint>
#include <iostream>

static constexpr size_t INIT_SIZE = 1 << 3;

namespace abyss {

ABYSS_INLINE uint64_t next_pow2(uint64_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

template <typename K, typename V>
ConcurrentHashMap<K, V>::ConcurrentHashMap(size_t size) {
    max_load_factor_ = 0.75f;
    table_ = Table<K, V>::init(std::max<size_t>(next_pow2(size), INIT_SIZE), max_load_factor_);
}

template <typename K, typename V>
ConcurrentHashMap<K, V>::~ConcurrentHashMap() {
    clear();
}

template <typename K, typename V>
void ConcurrentHashMap<K, V>::resize(size_t new_size) {
    Table<K, V> *new_table = table_->next.load(std::memory_order_acquire);

    // Create resize table if it doesn't exist.
    // May need a short mutex instead of CAS.
    if (new_table == nullptr) {
        // table_->next.compare_exchange_strong(new_table, (void *)MOVED_HASH);
        if (new_table == nullptr) {
            new_table = Table<K, V>::init(new_size, max_load_factor_);
            table_->next.store(new_table, std::memory_order_release);
        }
    }

    for (size_t i = 0; i < table_->size; ++i) {
        Entry<K, V> *entry = &table_->get_cells()[i];
        for (;;) {
            size_t hash = entry->hash.load(std::memory_order_relaxed);
            if (hash == NULL_HASH || hash == TOMBSTONE_HASH) {
                if (entry->hash.compare_exchange_strong(hash, MOVED_HASH)) break;
            } else if (hash == MOVED_HASH) {
                break;
            } else {

                // Entry<K, V> *new_entry = &new_table->get_cells()[hash & (new_table->size - 1)];
                // new_table->insert(entry->key.load(std::memory_order_relaxed),
                //                   entry->value.load(std::memory_order_relaxed));
                break;
            }
        }
    }
}

template <typename K, typename V>
void ConcurrentHashMap<K, V>::insert(const K &key, const V &value) {
    if (table_->slots_remaining < 0) {
        std::cerr << "Table is full!" << std::endl;
        resize(table_->size << 1);
    }

    const size_t key_hash = hash(key);
    const size_t size_mask = table_->size - 1;

    for (size_t idx = key_hash;; ++idx) {
        idx &= size_mask;

        size_t probed_hash = table_->get_cells()[idx].hash.load(std::memory_order_relaxed);
        if (probed_hash == NULL_HASH) {
            table_->get_cells()[idx].hash.compare_exchange_strong(probed_hash, key_hash);
            if (probed_hash != NULL_HASH) continue;  // Slot was taken by another thread.
            table_->get_cells()[idx].value.store(value, std::memory_order_relaxed);
            table_->slots_remaining--;
            return;
        } else if (probed_hash == key_hash) {
            table_->get_cells()[idx].value.store(value, std::memory_order_relaxed);
            return;
        }
    }
}

template <typename K, typename V>
void ConcurrentHashMap<K, V>::erase(const K &key) {

}

template <typename K, typename V>
V ConcurrentHashMap<K, V>::find(const K &key) const {
    const size_t key_hash = hash(key);
    const size_t size_mask = table_->size - 1;
    for (size_t idx = key_hash;; ++idx) {
        idx &= size_mask;

        size_t probed_hash = table_->get_cells()[idx].hash.load(std::memory_order_relaxed);
        if (probed_hash == key_hash) {
            return table_->get_cells()[idx].value.load(std::memory_order_relaxed);
        } else if (probed_hash == NULL_HASH) {
            return V();
        }
    }
}

template <typename K, typename V>
bool ConcurrentHashMap<K, V>::contains(const K &key) const {
    const size_t key_hash = hash(key);
    const size_t size_mask = table_->size - 1;
    for (size_t idx = key_hash;; ++idx) {
        idx &= size_mask;

        size_t probed_hash = table_->get_cells()[idx].hash.load(std::memory_order_relaxed);
        if (probed_hash == key_hash || probed_hash == MOVED_HASH) {
            return true;
        } else if (probed_hash == NULL_HASH) {
            return false;
        }
    }
}

};  // namespace abyss
