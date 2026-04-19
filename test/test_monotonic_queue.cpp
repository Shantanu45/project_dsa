#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <random>
#include "../src/algo/monotonic_queue.h"
#include "../src/dsa_framework/framework.h"

// --- helpers -----------------------------------------------------------------
static std::vector<int> random_vec(int n, unsigned seed = 42)
{
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> d(0, 100000);
    std::vector<int> v(n);
    std::generate(v.begin(), v.end(), [&] { return d(rng); });
    return v;
}

// =============================================================================
//  Sliding Window Maximum
// =============================================================================
TEST_CASE("SlidingWindowMax - correctness", "[monoqueue][correctness]")
{
    DSALog::info("[monoqueue] SlidingWindowMax deque vs naive");
    MonotonicQueue::SlidingWindowMaxDeque fast;
    MonotonicQueue::SlidingWindowMaxNaive naive;

    SECTION("LeetCode example")
    {
        MonotonicQueue::SlidingInput in{{1, 3, -1, -3, 5, 3, 6, 7}, 3};
        auto expected = std::vector<int>{3, 3, 5, 5, 6, 7};
        REQUIRE(fast.run(in)  == expected);
        REQUIRE(naive.run(in) == expected);
    }

    SECTION("k == 1: every element is its own max")
    {
        MonotonicQueue::SlidingInput in{{5, 1, 3, 2}, 1};
        REQUIRE(fast.run(in) == std::vector<int>{5, 1, 3, 2});
    }

    SECTION("k == n: single result is the global max")
    {
        MonotonicQueue::SlidingInput in{{3, 1, 4, 1, 5, 9, 2, 6}, 8};
        REQUIRE(fast.run(in)  == std::vector<int>{9});
        REQUIRE(naive.run(in) == std::vector<int>{9});
    }

    SECTION("sorted ascending")
    {
        MonotonicQueue::SlidingInput in{{1, 2, 3, 4, 5}, 2};
        REQUIRE(fast.run(in) == std::vector<int>{2, 3, 4, 5});
    }

    SECTION("sorted descending")
    {
        MonotonicQueue::SlidingInput in{{5, 4, 3, 2, 1}, 2};
        REQUIRE(fast.run(in) == std::vector<int>{5, 4, 3, 2});
    }

    SECTION("all equal")
    {
        MonotonicQueue::SlidingInput in{{7, 7, 7, 7, 7}, 3};
        REQUIRE(fast.run(in) == std::vector<int>{7, 7, 7});
    }

    SECTION("single element array")
    {
        MonotonicQueue::SlidingInput in{{42}, 1};
        REQUIRE(fast.run(in) == std::vector<int>{42});
    }

    SECTION("deque and naive always agree on random input")
    {
        auto arr = random_vec(500);
        for (int k : {1, 5, 10, 50, 100, 500})
        {
            MonotonicQueue::SlidingInput in{arr, k};
            REQUIRE(fast.run(in) == naive.run(in));
        }
    }
}

// =============================================================================
//  Sliding Window Minimum
// =============================================================================
TEST_CASE("SlidingWindowMin - correctness", "[monoqueue][correctness]")
{
    DSALog::info("[monoqueue] SlidingWindowMin");
    MonotonicQueue::SlidingWindowMinDeque min_algo;

    SECTION("basic min window")
    {
        MonotonicQueue::SlidingInput in{{1, 3, -1, -3, 5, 3, 6, 7}, 3};
        auto expected = std::vector<int>{-1, -3, -3, -3, 3, 3};
        REQUIRE(min_algo.run(in) == expected);
    }

    SECTION("k == 1 returns original array")
    {
        MonotonicQueue::SlidingInput in{{4, 2, 6, 1}, 1};
        REQUIRE(min_algo.run(in) == std::vector<int>{4, 2, 6, 1});
    }

    SECTION("sorted ascending: min is always first of window")
    {
        MonotonicQueue::SlidingInput in{{1, 2, 3, 4, 5}, 3};
        REQUIRE(min_algo.run(in) == std::vector<int>{1, 2, 3});
    }
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("MonotonicQueue benchmarks", "[monoqueue][benchmark][!benchmark]")
{
    auto arr1k  = random_vec(1000);
    auto arr10k = random_vec(10000);

    BENCHMARK("MonotonicDeque  1k k=10")   { return MonotonicQueue::SlidingWindowMaxDeque{}.run({arr1k,  10}); };
    BENCHMARK("NaiveSliding    1k k=10")   { return MonotonicQueue::SlidingWindowMaxNaive{}.run({arr1k,  10}); };
    BENCHMARK("MonotonicDeque 10k k=100")  { return MonotonicQueue::SlidingWindowMaxDeque{}.run({arr10k, 100}); };
    BENCHMARK("NaiveSliding   10k k=100")  { return MonotonicQueue::SlidingWindowMaxNaive{}.run({arr10k, 100}); };
    BENCHMARK("MonotonicDeque 10k k=1000") { return MonotonicQueue::SlidingWindowMaxDeque{}.run({arr10k, 1000}); };
    BENCHMARK("NaiveSliding   10k k=1000") { return MonotonicQueue::SlidingWindowMaxNaive{}.run({arr10k, 1000}); };
}
