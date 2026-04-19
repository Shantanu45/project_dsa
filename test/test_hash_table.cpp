#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <random>
#include "../src/algo/hash_table.h"
#include "../src/dsa_framework/framework.h"

// --- helpers -----------------------------------------------------------------
static std::vector<std::pair<int, int>> random_kv(int n, unsigned seed = 42)
{
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> d(0, 1'000'000);
    std::vector<std::pair<int, int>> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i) v.push_back({d(rng), d(rng)});
    return v;
}

// =============================================================================
//  ChainingHashMap
// =============================================================================
TEST_CASE("ChainingHashMap - basic operations", "[hashtable][correctness]")
{
    DSALog::info("[hashtable] ChainingHashMap basic ops");
    HashTable::ChainingHashMap<int, int> m;

    SECTION("empty map")
    {
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
        REQUIRE_FALSE(m.contains(1));
        REQUIRE(m.get(1) == nullptr);
    }

    SECTION("insert and get")
    {
        m.insert(1, 100);
        m.insert(2, 200);
        REQUIRE(m.size() == 2);
        REQUIRE(m.contains(1));
        REQUIRE(*m.get(1) == 100);
        REQUIRE(*m.get(2) == 200);
        REQUIRE(m.get(3) == nullptr);
    }

    SECTION("update overwrites existing key")
    {
        m.insert(5, 50);
        m.insert(5, 99);
        REQUIRE(m.size() == 1);
        REQUIRE(*m.get(5) == 99);
    }

    SECTION("remove")
    {
        m.insert(3, 30);
        REQUIRE(m.remove(3));
        REQUIRE_FALSE(m.contains(3));
        REQUIRE(m.size() == 0);
        REQUIRE_FALSE(m.remove(99));  // non-existent is no-op
    }

    SECTION("many inserts trigger rehash and preserve all values")
    {
        for (int i = 0; i < 100; ++i) m.insert(i, i * 10);
        REQUIRE(m.size() == 100);
        for (int i = 0; i < 100; ++i) REQUIRE(*m.get(i) == i * 10);
    }
}

// =============================================================================
//  OpenAddressHashMap
// =============================================================================
TEST_CASE("OpenAddressHashMap - basic operations", "[hashtable][correctness]")
{
    DSALog::info("[hashtable] OpenAddressHashMap basic ops");
    HashTable::OpenAddressHashMap<int, int> m;

    SECTION("empty map")
    {
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
        REQUIRE_FALSE(m.contains(42));
        REQUIRE(m.get(42) == nullptr);
    }

    SECTION("insert and get")
    {
        m.insert(10, 1);
        m.insert(20, 2);
        REQUIRE(m.size() == 2);
        REQUIRE(m.contains(10));
        REQUIRE(*m.get(10) == 1);
        REQUIRE(*m.get(20) == 2);
    }

    SECTION("update overwrites existing key")
    {
        m.insert(7, 70);
        m.insert(7, 77);
        REQUIRE(m.size() == 1);
        REQUIRE(*m.get(7) == 77);
    }

    SECTION("remove with tombstone: re-insert after remove")
    {
        m.insert(5, 50);
        REQUIRE(m.remove(5));
        REQUIRE_FALSE(m.contains(5));
        m.insert(5, 55);   // must be findable through the tombstone slot
        REQUIRE(*m.get(5) == 55);
        REQUIRE(m.size() == 1);
    }

    SECTION("many inserts trigger rehash and preserve all values")
    {
        for (int i = 0; i < 100; ++i) m.insert(i, i * 2);
        REQUIRE(m.size() == 100);
        for (int i = 0; i < 100; ++i) REQUIRE(*m.get(i) == i * 2);
    }
}

// =============================================================================
//  ChainingMap vs OpenAddrMap via algorithm wrappers
// =============================================================================
TEST_CASE("ChainMap and OpenMap produce same results", "[hashtable][correctness]")
{
    DSALog::info("[hashtable] ChainingMap vs OpenAddressMap");
    HashTable::ChainMapSearch chain;
    HashTable::OpenMapSearch  open;

    SECTION("all queries hit")
    {
        HashTable::MapInput in{{{1, 10}, {2, 20}, {3, 30}}, {1, 2, 3}};
        REQUIRE(chain.run(in) == std::vector<int>{10, 20, 30});
        REQUIRE(open.run(in)  == std::vector<int>{10, 20, 30});
    }

    SECTION("all queries miss")
    {
        HashTable::MapInput in{{{1, 10}}, {5, 6, 7}};
        REQUIRE(chain.run(in) == std::vector<int>{-1, -1, -1});
        REQUIRE(open.run(in)  == std::vector<int>{-1, -1, -1});
    }

    SECTION("mixed hit and miss")
    {
        HashTable::MapInput in{{{10, 100}, {20, 200}}, {10, 99, 20}};
        auto expected = std::vector<int>{100, -1, 200};
        REQUIRE(chain.run(in) == expected);
        REQUIRE(open.run(in)  == expected);
    }

    SECTION("large random input: both maps agree")
    {
        auto kv = random_kv(500);
        // deduplicate keys so both maps see the same final state
        std::sort(kv.begin(), kv.end(), [](auto& a, auto& b){ return a.first < b.first; });
        kv.erase(std::unique(kv.begin(), kv.end(), [](auto& a, auto& b){ return a.first == b.first; }), kv.end());
        std::vector<int> queries;
        for (auto& [k, v] : kv) queries.push_back(k);
        HashTable::MapInput in{kv, queries};
        REQUIRE(chain.run(in) == open.run(in));
    }
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("HashTable benchmarks", "[hashtable][benchmark][!benchmark]")
{
    auto kv1k  = random_kv(1000);
    auto kv10k = random_kv(10000);

    std::vector<int> q1k, q10k;
    for (auto& [k, v] : kv1k)  q1k.push_back(k);
    for (auto& [k, v] : kv10k) q10k.push_back(k);

    HashTable::MapInput in1k{kv1k, q1k};
    HashTable::MapInput in10k{kv10k, q10k};

    BENCHMARK("ChainingMap   1k") { return HashTable::ChainMapSearch{}.run(in1k); };
    BENCHMARK("OpenAddrMap   1k") { return HashTable::OpenMapSearch{}.run(in1k); };
    BENCHMARK("ChainingMap  10k") { return HashTable::ChainMapSearch{}.run(in10k); };
    BENCHMARK("OpenAddrMap  10k") { return HashTable::OpenMapSearch{}.run(in10k); };
}
