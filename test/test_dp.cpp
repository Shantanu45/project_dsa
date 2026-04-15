#include "../src/algo/dp.h"
#include "../src/dsa_framework/framework.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>

// --- Knapsack -----------------------------------------------------------------
TEST_CASE("Knapsack 0/1 - correctness", "[dp][knapsack][correctness]")
{
  DP::Knapsack ks;

  SECTION("classic 4-item example") { REQUIRE(ks.run({ 10, { 2, 3, 4, 5 }, { 3, 4, 5, 6 } }) == 13); }
  SECTION("single item fits") { REQUIRE(ks.run({ 5, { 3 }, { 4 } }) == 4); }
  SECTION("single item too heavy") { REQUIRE(ks.run({ 2, { 3 }, { 4 } }) == 0); }
  SECTION("zero capacity") { REQUIRE(ks.run({ 0, { 1, 2, 3 }, { 10, 20, 30 } }) == 0); }
  SECTION("all items fit") { REQUIRE(ks.run({ 100, { 1, 2, 3 }, { 10, 20, 30 } }) == 60); }
  SECTION("no items") { REQUIRE(ks.run({ 10, {}, {} }) == 0); }
}

// --- LCS ---------------------------------------------------------------------
TEST_CASE("LCS - correctness", "[dp][lcs][correctness]")
{
  DP::LCS lcs;

  REQUIRE(lcs.run({ "AGGTAB", "GXTXAYB" }) == 4);
  REQUIRE(lcs.run({ "ABCBDAB", "BDCABA" }) == 4);
  REQUIRE(lcs.run({ "", "ABC" }) == 0);
  REQUIRE(lcs.run({ "ABC", "" }) == 0);
  REQUIRE(lcs.run({ "ABC", "ABC" }) == 3);
  REQUIRE(lcs.run({ "A", "B" }) == 0);
  REQUIRE(lcs.run({ "ABC", "AC" }) == 2);
}

// --- Edit Distance ------------------------------------------------------------
TEST_CASE("EditDistance - correctness", "[dp][editdistance][correctness]")
{
  DP::EditDistance ed;

  REQUIRE(ed.run({ "kitten", "sitting" }) == 3);
  REQUIRE(ed.run({ "sunday", "saturday" }) == 3);
  REQUIRE(ed.run({ "", "abc" }) == 3);
  REQUIRE(ed.run({ "abc", "" }) == 3);
  REQUIRE(ed.run({ "abc", "abc" }) == 0);
  REQUIRE(ed.run({ "a", "b" }) == 1);
}

// --- Coin Change -------------------------------------------------------------
TEST_CASE("CoinChange - min coins", "[dp][coinchange][correctness]")
{
  DP::CoinChange cc;

  REQUIRE(cc.run({ { 1, 2, 5 }, 11 }) == 3);// 5+5+1
  REQUIRE(cc.run({ { 2 }, 3 }) == -1);// impossible
  REQUIRE(cc.run({ { 1 }, 0 }) == 0);// zero amount
  REQUIRE(cc.run({ { 1 }, 5 }) == 5);// only pennies
  REQUIRE(cc.run({ { 1, 5, 6, 9 }, 11 }) == 2);// 2×6 beats 5+6 or 9+1+1
}

// --- property: LCS <= min(len(a), len(b)) ------------------------------------
TEST_CASE("LCS property - length bound", "[dp][lcs][property]")
{
  DP::LCS lcs;
  std::vector<std::pair<std::string, std::string>> pairs = {
    { "ABCDE", "ACE" }, { "XYZ", "XYZ" }, { "HELLO", "WORLD" }, { "", "TEST" }
  };
  for (auto &[a, b] : pairs) {
    int result = lcs.run({ a, b });
    REQUIRE(result >= 0);
    REQUIRE(result <= (int)std::min(a.size(), b.size()));
  }
}

// --- benchmarks ---------------------------------------------------------------
TEST_CASE("DP benchmarks", "[dp][benchmark][!benchmark]")
{
  DP::KnapsackInput big_ks{ 500, std::vector<int>(50, 10), std::vector<int>(50, 20) };
  for (int i = 0; i < 50; ++i) {
    big_ks.weights[i] = i + 1;
    big_ks.values[i] = (i + 1) * 3;
  }

  BENCHMARK("Knapsack  50-items cap=500") { return DP::Knapsack{}.run(big_ks); };
  BENCHMARK("LCS       len=100")
  {
    std::string a(100, 'A'), b(100, 'B');
    for (int i = 0; i < 100; i += 3) b[i] = 'A';
    return DP::LCS{}.run({ a, b });
  };
  BENCHMARK("EditDist  len=50")
  {
    return DP::EditDistance{}.run({ "abcdefghijklmnopqrstuvwxyz", "zyxwvutsrqponmlkjihgfedcba" });
  };
  BENCHMARK("CoinChange amount=10000") { return DP::CoinChange{}.run({ { 1, 5, 10, 25, 50 }, 10000 }); };
}