#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <numeric>
#include <random>
#include "../src/algo/segment_tree.h"
#include "../src/dsa_framework/framework.h"

// --- helpers -----------------------------------------------------------------
static std::vector<int> random_vec(int n, unsigned seed = 42)
{
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> d(0, 1000);
    std::vector<int> v(n);
    std::generate(v.begin(), v.end(), [&] { return d(rng); });
    return v;
}

// =============================================================================
//  Range Sum
// =============================================================================
TEST_CASE("SegmentTree - range sum", "[segtree][correctness]")
{
    DSALog::info("[segtree] RangeSum tree vs naive");
    SegTree::RangeSumTree  tree_algo;
    SegTree::RangeSumNaive naive_algo;

    SECTION("full range sum")
    {
        SegTree::RangeInput in{{1, 2, 3, 4, 5}, {{0, 4}}, {}};
        REQUIRE(tree_algo.run(in)  == std::vector<int>{15});
        REQUIRE(naive_algo.run(in) == std::vector<int>{15});
    }

    SECTION("single element range")
    {
        SegTree::RangeInput in{{10, 20, 30}, {{1, 1}}, {}};
        REQUIRE(tree_algo.run(in) == std::vector<int>{20});
    }

    SECTION("multiple non-overlapping queries")
    {
        SegTree::RangeInput in{{1, 2, 3, 4, 5}, {{0, 2}, {1, 3}, {2, 4}}, {}};
        auto expected = std::vector<int>{6, 9, 12};
        REQUIRE(tree_algo.run(in)  == expected);
        REQUIRE(naive_algo.run(in) == expected);
    }

    SECTION("point update then query")
    {
        // arr = {1,2,3,4,5}; update arr[2]=10; sum[0..4] = 1+2+10+4+5 = 22
        SegTree::RangeInput in{{1, 2, 3, 4, 5}, {{0, 4}}, {{2, 10}}};
        REQUIRE(tree_algo.run(in)  == std::vector<int>{22});
        REQUIRE(naive_algo.run(in) == std::vector<int>{22});
    }

    SECTION("tree and naive always agree on random input")
    {
        auto arr = random_vec(100);
        std::vector<SegTree::RangeQuery> qs;
        for (int i = 0; i + 5 <= 100; i += 5) qs.push_back({i, i + 4});
        SegTree::RangeInput in{arr, qs, {}};
        REQUIRE(tree_algo.run(in) == naive_algo.run(in));
    }
}

// =============================================================================
//  Range Min
// =============================================================================
TEST_CASE("SegmentTree - range min", "[segtree][correctness]")
{
    DSALog::info("[segtree] RangeMin tree vs naive");
    SegTree::RangeMinTree  tree_algo;
    SegTree::RangeMinNaive naive_algo;

    SECTION("global min over full range")
    {
        SegTree::RangeInput in{{3, 1, 4, 1, 5, 9, 2, 6}, {{0, 7}}, {}};
        REQUIRE(tree_algo.run(in)  == std::vector<int>{1});
        REQUIRE(naive_algo.run(in) == std::vector<int>{1});
    }

    SECTION("multiple subrange queries")
    {
        SegTree::RangeInput in{{5, 3, 7, 1, 8, 2}, {{0, 2}, {1, 4}, {3, 5}}, {}};
        auto expected = std::vector<int>{3, 1, 1};
        REQUIRE(tree_algo.run(in)  == expected);
        REQUIRE(naive_algo.run(in) == expected);
    }

    SECTION("min drops after point update")
    {
        // arr[1] = 0 -> min over [0..2] drops from 3 to 0
        SegTree::RangeInput in{{5, 3, 7}, {{0, 2}}, {{1, 0}}};
        REQUIRE(tree_algo.run(in)  == std::vector<int>{0});
        REQUIRE(naive_algo.run(in) == std::vector<int>{0});
    }

    SECTION("tree and naive agree on random input")
    {
        auto arr = random_vec(200);
        std::vector<SegTree::RangeQuery> qs;
        for (int i = 0; i + 10 <= 200; i += 10) qs.push_back({i, i + 9});
        SegTree::RangeInput in{arr, qs, {}};
        REQUIRE(tree_algo.run(in) == naive_algo.run(in));
    }
}

// =============================================================================
//  Range Max
// =============================================================================
TEST_CASE("SegmentTree - range max", "[segtree][correctness]")
{
    DSALog::info("[segtree] RangeMax tree vs naive");
    SegTree::RangeMaxTree  tree_algo;
    SegTree::RangeMaxNaive naive_algo;

    SECTION("global max")
    {
        SegTree::RangeInput in{{3, 1, 4, 1, 5, 9, 2, 6}, {{0, 7}}, {}};
        REQUIRE(tree_algo.run(in)  == std::vector<int>{9});
        REQUIRE(naive_algo.run(in) == std::vector<int>{9});
    }

    SECTION("tree and naive agree on random input")
    {
        auto arr = random_vec(200);
        std::vector<SegTree::RangeQuery> qs;
        for (int i = 0; i + 10 <= 200; i += 10) qs.push_back({i, i + 9});
        SegTree::RangeInput in{arr, qs, {}};
        REQUIRE(tree_algo.run(in) == naive_algo.run(in));
    }
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("SegmentTree benchmarks", "[segtree][benchmark][!benchmark]")
{
    auto arr = random_vec(10000);
    std::vector<SegTree::RangeQuery> qs;
    for (int i = 0; i + 100 <= 10000; i += 10) qs.push_back({i, i + 99});

    SegTree::RangeInput in{arr, qs, {}};

    BENCHMARK("RangeSumTree  10k/1k")  { return SegTree::RangeSumTree{}.run(in); };
    BENCHMARK("RangeSumNaive 10k/1k")  { return SegTree::RangeSumNaive{}.run(in); };
    BENCHMARK("RangeMinTree  10k/1k")  { return SegTree::RangeMinTree{}.run(in); };
    BENCHMARK("RangeMinNaive 10k/1k")  { return SegTree::RangeMinNaive{}.run(in); };
    BENCHMARK("RangeMaxTree  10k/1k")  { return SegTree::RangeMaxTree{}.run(in); };
    BENCHMARK("RangeMaxNaive 10k/1k")  { return SegTree::RangeMaxNaive{}.run(in); };
}
