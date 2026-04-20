/*****************************************************************//**
 * \file   hash_table.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <functional>
#include <list>
#include <utility>
#include <vector>
#include "dsa_framework/framework.h"

namespace HashTable {

// =============================================================================
//  ChainingHashMap<K,V>  –  separate chaining with std::list buckets
// =============================================================================
/**
 * Each bucket is a linked list of (K,V) pairs.
 * Collisions are resolved by appending to the chain at bucket_index(k).
 *
 * Complexities (n = elements, b = bucket count):
 *   insert / get / remove  O(1) average,  O(n) worst (all keys hash to one bucket)
 *   load_factor = n/b;  rehash doubles buckets when load > 0.75
 *
 * Cache behaviour: pointer-chasing on every lookup – slower than open addressing
 * for small values despite the same Big-O.
 */
template <typename K, typename V>
class ChainingHashMap
{
    static constexpr double MAX_LOAD    = 0.75;
    static constexpr int    INIT_BUCKETS = 16;

    using Bucket = std::list<std::pair<K, V>>;

    std::vector<Bucket> buckets_;
    size_t              size_;

    size_t bucket_idx(const K& k) const
    {
        return std::hash<K>{}(k) % buckets_.size();
    }

    void rehash()
    {
        std::vector<Bucket> old = std::move(buckets_);
        buckets_.assign(old.size() * 2, Bucket{});
        size_ = 0;
        for (auto& b : old)
            for (auto& [k, v] : b)
                insert(k, v);
    }

   public:
    explicit ChainingHashMap(int buckets = INIT_BUCKETS) : buckets_(buckets), size_(0) {}

    void insert(const K& k, const V& v)
    {
        if (load_factor() > MAX_LOAD) rehash();
        auto& b = buckets_[bucket_idx(k)];
        for (auto& [ek, ev] : b)
        {
            if (ek == k) { ev = v; return; }
        }
        b.emplace_back(k, v);
        ++size_;
    }

    bool contains(const K& k) const
    {
        for (auto& [ek, ev] : buckets_[bucket_idx(k)])
            if (ek == k) return true;
        return false;
    }

    V* get(const K& k)
    {
        for (auto& [ek, ev] : buckets_[bucket_idx(k)])
            if (ek == k) return &ev;
        return nullptr;
    }

    bool remove(const K& k)
    {
        auto& b = buckets_[bucket_idx(k)];
        for (auto it = b.begin(); it != b.end(); ++it)
        {
            if (it->first == k) { b.erase(it); --size_; return true; }
        }
        return false;
    }

    size_t size()         const { return size_; }
    bool   empty()        const { return size_ == 0; }
    double load_factor()  const { return (double)size_ / buckets_.size(); }
    size_t bucket_count() const { return buckets_.size(); }
};

// =============================================================================
//  OpenAddressHashMap<K,V>  –  linear probing with tombstone deletion
// =============================================================================
/**
 * All entries live in a single flat array — much more cache-friendly than
 * chaining.  Deletion uses a DELETED tombstone so probe sequences stay intact.
 *
 * Complexities:
 *   insert / get / remove  O(1) average
 *   rehash triggered at load > 0.5 (lower threshold than chaining to keep
 *   probe chains short; tombstones count toward the threshold)
 *
 * Key insight: after remove() the slot is marked DELETED, not EMPTY, so
 * subsequent lookups don't stop early at that position.
 */
template <typename K, typename V>
class OpenAddressHashMap
{
    static constexpr double MAX_LOAD   = 0.5;
    static constexpr int    INIT_CAP   = 16;

    enum class State { EMPTY, OCCUPIED, DELETED };
    struct Slot { K key{}; V val{}; State state = State::EMPTY; };

    std::vector<Slot> table_;
    size_t            size_;
    size_t            deleted_;

    void rehash()
    {
        std::vector<Slot> old = std::move(table_);
        table_.assign(old.size() * 2, Slot{});
        size_ = deleted_ = 0;
        for (auto& s : old)
            if (s.state == State::OCCUPIED)
                insert(s.key, s.val);
    }

   public:
    explicit OpenAddressHashMap(int cap = INIT_CAP) : table_(cap), size_(0), deleted_(0) {}

    void insert(const K& k, const V& v)
    {
        if ((double)(size_ + deleted_) / table_.size() > MAX_LOAD) rehash();
        size_t h           = std::hash<K>{}(k) % table_.size();
        size_t insert_pos  = SIZE_MAX;
        size_t i           = h;
        do
        {
            if (table_[i].state == State::EMPTY)
            {
                if (insert_pos == SIZE_MAX) insert_pos = i;
                break;
            }
            if (table_[i].state == State::DELETED)
            {
                if (insert_pos == SIZE_MAX) insert_pos = i;
            }
            else if (table_[i].key == k)
            {
                table_[i].val = v; return;
            }
            i = (i + 1) % table_.size();
        } while (i != h);

        table_[insert_pos] = {k, v, State::OCCUPIED};
        ++size_;
    }

    bool contains(const K& k) const
    {
        size_t h = std::hash<K>{}(k) % table_.size();
        size_t i = h;
        do
        {
            if (table_[i].state == State::EMPTY) return false;
            if (table_[i].state == State::OCCUPIED && table_[i].key == k) return true;
            i = (i + 1) % table_.size();
        } while (i != h);
        return false;
    }

    V* get(const K& k)
    {
        size_t h = std::hash<K>{}(k) % table_.size();
        size_t i = h;
        do
        {
            if (table_[i].state == State::EMPTY) return nullptr;
            if (table_[i].state == State::OCCUPIED && table_[i].key == k) return &table_[i].val;
            i = (i + 1) % table_.size();
        } while (i != h);
        return nullptr;
    }

    bool remove(const K& k)
    {
        size_t h = std::hash<K>{}(k) % table_.size();
        size_t i = h;
        do
        {
            if (table_[i].state == State::EMPTY) return false;
            if (table_[i].state == State::OCCUPIED && table_[i].key == k)
            {
                table_[i].state = State::DELETED;
                --size_; ++deleted_; return true;
            }
            i = (i + 1) % table_.size();
        } while (i != h);
        return false;
    }

    size_t size()        const { return size_; }
    bool   empty()       const { return size_ == 0; }
    double load_factor() const { return (double)size_ / table_.size(); }
};

// =============================================================================
//  Algorithm wrappers
// =============================================================================
struct MapInput
{
    std::vector<std::pair<int, int>> entries;  // key-value pairs to insert
    std::vector<int>                 queries;  // keys to look up; -1 = not found
};
using MapOutput = std::vector<int>;

/**
 * @brief Algorithm wrapper: key-value search using separate-chaining hash map.
 *
 * @details
 * Inserts all (key, value) pairs from MapInput::entries into a ChainingHashMap,
 * then answers each query by looking up the key.  Returns the associated value,
 * or -1 for a miss.
 *
 * @par Complexity
 *   Build  O(n) average.   Query  O(1) average, O(n) worst.
 *   Higher constant than open addressing due to pointer-chasing through linked
 *   list buckets — each lookup potentially dereferences multiple heap pointers.
 */
struct ChainMapSearch : Algorithm<MapInput, MapOutput>
{
    MapOutput run(const MapInput& in) override
    {
        ChainingHashMap<int, int> m;
        for (auto& [k, v] : in.entries) m.insert(k, v);
        MapOutput out;
        out.reserve(in.queries.size());
        for (int q : in.queries)
        {
            auto* p = m.get(q);
            out.push_back(p ? *p : -1);
        }
        return out;
    }
    std::string name()       const override { return "ChainingMap"; }
    std::string complexity() const override { return "O(1) avg, pointer-heavy"; }
};

/**
 * @brief Algorithm wrapper: key-value search using open-addressing hash map.
 *
 * @details
 * Inserts all entries into an OpenAddressHashMap (linear probing), then answers
 * each query.  Returns the associated value, or -1 for a miss.
 *
 * @par Complexity
 *   Build  O(n) average.   Query  O(1) average.
 *   Better cache behaviour than ChainingHashMap: all data lives in one flat
 *   array, so lookups hit L1/L2 cache much more frequently.
 *   Tombstones (DELETED slots) can accumulate and slow down probing — periodic
 *   rehash is triggered at load > 0.5 to keep probe chains short.
 */
struct OpenMapSearch : Algorithm<MapInput, MapOutput>
{
    MapOutput run(const MapInput& in) override
    {
        OpenAddressHashMap<int, int> m;
        for (auto& [k, v] : in.entries) m.insert(k, v);
        MapOutput out;
        out.reserve(in.queries.size());
        for (int q : in.queries)
        {
            auto* p = m.get(q);
            out.push_back(p ? *p : -1);
        }
        return out;
    }
    std::string name()       const override { return "OpenAddrMap"; }
    std::string complexity() const override { return "O(1) avg, cache-friendly"; }
};

}  // namespace HashTable
