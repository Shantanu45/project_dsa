#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <algorithm>
#include <random>
#include <vector>
#include "../src/algo/bit_manip.h"

// --- count_bits_kernighan / count_bits_popcount --------------------------------

TEST_CASE("count_bits - known values", "[bitmanip][correctness]")
{
    using namespace BitManip;
    SECTION("zero has no set bits")
    {
        REQUIRE(count_bits_kernighan(0) == 0);
        REQUIRE(count_bits_popcount(0)  == 0);
    }
    SECTION("powers of two have exactly one set bit")
    {
        for (int i = 0; i < 32; ++i)
        {
            uint32_t v = 1u << i;
            REQUIRE(count_bits_kernighan(v) == 1);
            REQUIRE(count_bits_popcount(v)  == 1);
        }
    }
    SECTION("all-ones 32-bit value has 32 set bits")
    {
        REQUIRE(count_bits_kernighan(0xFFFFFFFFu) == 32);
        REQUIRE(count_bits_popcount(0xFFFFFFFFu)  == 32);
    }
    SECTION("kernighan and popcount always agree on random input")
    {
        std::mt19937 rng(42);
        std::uniform_int_distribution<uint32_t> dist(0u, 0xFFFFFFFFu);
        for (int i = 0; i < 500; ++i)
        {
            uint32_t v = dist(rng);
            REQUIRE(count_bits_kernighan(v) == count_bits_popcount(v));
        }
    }
}

// --- is_power_of_two -----------------------------------------------------------

TEST_CASE("is_power_of_two - correctness", "[bitmanip][correctness]")
{
    using namespace BitManip;
    SECTION("zero is not a power of two")  { REQUIRE_FALSE(is_power_of_two(0)); }
    SECTION("1 is 2^0")                    { REQUIRE(is_power_of_two(1)); }
    SECTION("all powers 2^1 .. 2^31")
    {
        for (int i = 1; i < 32; ++i)
            REQUIRE(is_power_of_two(1u << i));
    }
    SECTION("non-powers return false")
    {
        for (uint32_t v : {3u, 5u, 6u, 7u, 9u, 10u, 100u, 1023u})
            REQUIRE_FALSE(is_power_of_two(v));
    }
}

// --- next_power_of_two ---------------------------------------------------------

TEST_CASE("next_power_of_two - correctness", "[bitmanip][correctness]")
{
    using namespace BitManip;
    SECTION("0 maps to 1")  { REQUIRE(next_power_of_two(0) == 1); }
    SECTION("1 maps to 1")  { REQUIRE(next_power_of_two(1) == 1); }
    SECTION("exact powers are unchanged")
    {
        for (int i = 1; i < 31; ++i)
        {
            uint32_t p = 1u << i;
            REQUIRE(next_power_of_two(p) == p);
        }
    }
    SECTION("non-powers round up")
    {
        REQUIRE(next_power_of_two(3)  == 4);
        REQUIRE(next_power_of_two(5)  == 8);
        REQUIRE(next_power_of_two(6)  == 8);
        REQUIRE(next_power_of_two(7)  == 8);
        REQUIRE(next_power_of_two(9)  == 16);
        REQUIRE(next_power_of_two(100) == 128);
    }
    SECTION("result is always a power of two")
    {
        for (uint32_t v = 1; v < 1024; ++v)
            REQUIRE(is_power_of_two(next_power_of_two(v)));
    }
    SECTION("result is always >= input")
    {
        for (uint32_t v = 0; v < 1024; ++v)
            REQUIRE(next_power_of_two(v) >= v);
    }
}

// --- reverse_bits --------------------------------------------------------------

TEST_CASE("reverse_bits - correctness", "[bitmanip][correctness]")
{
    using namespace BitManip;
    SECTION("0 reversed is 0")   { REQUIRE(reverse_bits(0) == 0); }
    SECTION("all-ones is fixed") { REQUIRE(reverse_bits(0xFFFFFFFFu) == 0xFFFFFFFFu); }
    SECTION("bit 0 becomes bit 31")
    {
        REQUIRE(reverse_bits(1u) == (1u << 31));
        REQUIRE(reverse_bits(1u << 31) == 1u);
    }
    SECTION("double reversal is identity")
    {
        for (uint32_t v : {0u, 1u, 0xABCDEF01u, 0x12345678u, 0xFFFFFFFFu})
            REQUIRE(reverse_bits(reverse_bits(v)) == v);
    }
    SECTION("bit count is preserved")
    {
        for (uint32_t v : {0u, 7u, 255u, 0xAAAAAAAAu, 0x12345678u})
            REQUIRE(count_bits_popcount(reverse_bits(v)) == count_bits_popcount(v));
    }
}

// --- single_number_xor ---------------------------------------------------------

TEST_CASE("single_number_xor - correctness", "[bitmanip][correctness]")
{
    using namespace BitManip;
    SECTION("single element in array")   { REQUIRE(single_number_xor({42}) == 42); }
    SECTION("classic 3-element example") { REQUIRE(single_number_xor({2, 1, 2}) == 1); }
    SECTION("larger array")
    {
        REQUIRE(single_number_xor({4, 1, 2, 1, 2}) == 4);
        REQUIRE(single_number_xor({1, 3, 1, 3, 5}) == 5);
    }
    SECTION("negative numbers")
    {
        REQUIRE(single_number_xor({-1, -2, -1}) == -2);
    }
}

// --- two_single_numbers --------------------------------------------------------

TEST_CASE("two_single_numbers - correctness", "[bitmanip][correctness]")
{
    using namespace BitManip;
    auto sorted_pair = [](std::pair<int,int> p) -> std::pair<int,int> {
        if (p.first > p.second) std::swap(p.first, p.second);
        return p;
    };

    SECTION("basic case")
    {
        auto [a, b] = sorted_pair(two_single_numbers({1, 2, 1, 3, 2, 5}));
        REQUIRE(a == 3);
        REQUIRE(b == 5);
    }
    SECTION("two elements only")
    {
        auto [a, b] = sorted_pair(two_single_numbers({7, 42}));
        REQUIRE(a == 7);
        REQUIRE(b == 42);
    }
    SECTION("negatives")
    {
        auto [a, b] = sorted_pair(two_single_numbers({-3, 1, 1, -3, -1, 4}));
        REQUIRE(a == -1);
        REQUIRE(b == 4);
    }
}

// --- Algorithm wrappers --------------------------------------------------------

TEST_CASE("PopcountKernighan and PopcountStd wrappers agree", "[bitmanip][correctness]")
{
    BitManip::PopcountKernighan kern;
    BitManip::PopcountStd       stdp;
    std::vector<uint32_t> vals = {0, 1, 2, 3, 255, 256, 0xDEADBEEFu, 0xFFFFFFFFu};
    auto a = kern.run({vals});
    auto b = stdp.run({vals});
    REQUIRE(a == b);
}

TEST_CASE("SingleNumberXOR and SingleNumberSort wrappers agree", "[bitmanip][correctness]")
{
    BitManip::SingleNumberXOR  xor_algo;
    BitManip::SingleNumberSort sort_algo;
    std::vector<std::vector<int>> cases = {
        {42},
        {1, 2, 1},
        {4, 1, 2, 1, 2},
        {-1, -2, -1},
    };
    for (auto& arr : cases)
    {
        int expected = xor_algo.run({arr});
        REQUIRE(sort_algo.run({arr}) == expected);
    }
}

// --- benchmarks ----------------------------------------------------------------

TEST_CASE("BitManip benchmarks", "[bitmanip][benchmark][!benchmark]")
{
    std::vector<uint32_t> vals(10000);
    for (int i = 0; i < 10000; ++i) vals[i] = static_cast<uint32_t>(i * 2654435761u);

    BENCHMARK("Kernighan popcount 10k values")
    {
        return BitManip::PopcountKernighan{}.run({vals});
    };
    BENCHMARK("std::popcount 10k values")
    {
        return BitManip::PopcountStd{}.run({vals});
    };
}
