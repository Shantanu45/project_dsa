#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <string>
#include <vector>
#include "../src/algo/string_algo.h"
#include "../src/dsa_framework/framework.h"

// =============================================================================
//  KMP
// =============================================================================
TEST_CASE("KMP - pattern search", "[stringalgo][correctness]")
{
    DSALog::info("[stringalgo] KMP search");
    StringAlgo::KMPSearch   kmp;
    StringAlgo::NaiveSearch naive;

    SECTION("single occurrence")
    {
        StringAlgo::SearchInput in{"hello world", "world"};
        REQUIRE(kmp.run(in)   == std::vector<int>{6});
        REQUIRE(naive.run(in) == std::vector<int>{6});
    }

    SECTION("multiple overlapping occurrences")
    {
        StringAlgo::SearchInput in{"aaaa", "aa"};
        auto expected = std::vector<int>{0, 1, 2};
        REQUIRE(kmp.run(in)   == expected);
        REQUIRE(naive.run(in) == expected);
    }

    SECTION("no match")
    {
        StringAlgo::SearchInput in{"hello", "xyz"};
        REQUIRE(kmp.run(in).empty());
        REQUIRE(naive.run(in).empty());
    }

    SECTION("pattern equals text")
    {
        StringAlgo::SearchInput in{"abc", "abc"};
        REQUIRE(kmp.run(in) == std::vector<int>{0});
    }

    SECTION("pattern longer than text")
    {
        StringAlgo::SearchInput in{"hi", "hello"};
        REQUIRE(kmp.run(in).empty());
    }

    SECTION("periodic pattern")
    {
        StringAlgo::SearchInput in{"ababababab", "abab"};
        auto expected = std::vector<int>{0, 2, 4, 6};
        REQUIRE(kmp.run(in)   == expected);
        REQUIRE(naive.run(in) == expected);
    }

    SECTION("pattern at very start and end")
    {
        StringAlgo::SearchInput in{"abXYZab", "ab"};
        auto expected = std::vector<int>{0, 5};
        REQUIRE(kmp.run(in) == expected);
    }
}

// =============================================================================
//  Z-Algorithm
// =============================================================================
TEST_CASE("Z-Algorithm - pattern search", "[stringalgo][correctness]")
{
    DSALog::info("[stringalgo] Z-algorithm search");
    StringAlgo::ZSearch   z;
    StringAlgo::KMPSearch kmp;

    SECTION("three occurrences")
    {
        StringAlgo::SearchInput in{"abcabcabc", "abc"};
        auto expected = std::vector<int>{0, 3, 6};
        REQUIRE(z.run(in)   == expected);
        REQUIRE(kmp.run(in) == expected);
    }

    SECTION("no match")
    {
        StringAlgo::SearchInput in{"abcdef", "xyz"};
        REQUIRE(z.run(in).empty());
    }

    SECTION("Z and KMP always agree")
    {
        std::string text = "aababcababcababcabc";
        for (const std::string& pat : {"a", "ab", "abc", "ababc", "xyz"})
        {
            StringAlgo::SearchInput in{text, pat};
            REQUIRE(z.run(in) == kmp.run(in));
        }
    }
}

// =============================================================================
//  All three algorithms agree
// =============================================================================
TEST_CASE("KMP, Z-Algorithm, Naive all agree", "[stringalgo][correctness]")
{
    DSALog::info("[stringalgo] KMP vs Z vs Naive");
    StringAlgo::KMPSearch   kmp;
    StringAlgo::ZSearch     z;
    StringAlgo::NaiveSearch naive;

    std::vector<std::pair<std::string, std::string>> cases = {
        {"mississippi",         "issi"      },
        {"aaaaaaaaaa",          "aaa"        },
        {"abcdef",              "cd"         },
        {"aabaacaadaabaaba",    "aabaa"      },
        {"THIS IS A TEST TEXT", "TEST"       },
    };

    for (auto& [text, pat] : cases)
    {
        StringAlgo::SearchInput in{text, pat};
        REQUIRE(kmp.run(in)   == naive.run(in));
        REQUIRE(z.run(in)     == naive.run(in));
    }
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("StringAlgo benchmarks", "[stringalgo][benchmark][!benchmark]")
{
    // Worst case for naive: all 'a's then one 'b' at the end
    std::string text(100000, 'a');
    text += 'b';
    std::string pat = std::string(20, 'a') + 'b';
    StringAlgo::SearchInput in_worst{text, pat};

    // Random-ish text
    std::string text2;
    for (int i = 0; i < 100000; ++i) text2 += (char)('a' + i % 26);
    StringAlgo::SearchInput in_rand{text2, "abcdefghij"};

    BENCHMARK("KMP    worst-case 100k") { return StringAlgo::KMPSearch{}.run(in_worst); };
    BENCHMARK("Z-Algo worst-case 100k") { return StringAlgo::ZSearch{}.run(in_worst); };
    BENCHMARK("Naive  worst-case 100k") { return StringAlgo::NaiveSearch{}.run(in_worst); };

    BENCHMARK("KMP    random 100k")     { return StringAlgo::KMPSearch{}.run(in_rand); };
    BENCHMARK("Z-Algo random 100k")     { return StringAlgo::ZSearch{}.run(in_rand); };
    BENCHMARK("Naive  random 100k")     { return StringAlgo::NaiveSearch{}.run(in_rand); };
}
