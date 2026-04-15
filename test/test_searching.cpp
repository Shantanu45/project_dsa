#include "../src/dsa_framework/framework.h"
#include "../src/algo/searching.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <numeric>

static std::vector<int> sorted_vec(int n)
{
  std::vector<int> v(n);
  std::iota(v.begin(), v.end(), 0);
  return v;
}

// --- correctness -------------------------------------------------------------
TEMPLATE_TEST_CASE("Search - found / not-found",
  "[searching][correctness]",
  Searching::BinarySearch,
  Searching::LinearSearch,
  Searching::InterpolationSearch)
{
  TestType algo;
  auto arr = sorted_vec(1000);// 0..999

  SECTION("find first element") { REQUIRE(algo.run({ arr, 0 }) == 0); }
  SECTION("find last element") { REQUIRE(algo.run({ arr, 999 }) == 999); }
  SECTION("find middle element") { REQUIRE(algo.run({ arr, 500 }) == 500); }
  SECTION("not found - above range") { REQUIRE(algo.run({ arr, 1000 }) == -1); }
  SECTION("not found - below range") { REQUIRE(algo.run({ arr, -1 }) == -1); }
  SECTION("empty array") { REQUIRE(algo.run({ {}, 5 }) == -1); }
  SECTION("single-element - hit") { REQUIRE(algo.run({ { 7 }, 7 }) == 0); }
  SECTION("single-element - miss") { REQUIRE(algo.run({ { 7 }, 3 }) == -1); }
}

TEST_CASE("BinarySearch - index correctness", "[searching][correctness]")
{
  Searching::BinarySearch bs;
  std::vector<int> arr = { 2, 5, 8, 12, 16, 23, 38, 56, 72, 91 };

  REQUIRE(bs.run({ arr, 2 }) == 0);
  REQUIRE(bs.run({ arr, 56 }) == 7);
  REQUIRE(bs.run({ arr, 91 }) == 9);
  REQUIRE(bs.run({ arr, 10 }) == -1);
}

// --- benchmarks --------------------------------------------------------------
TEST_CASE("Searching benchmarks", "[searching][benchmark][!benchmark]")
{
  auto arr1k = sorted_vec(1000);
  auto arr10k = sorted_vec(10000);

  BENCHMARK("BinarySearch    1k  mid") { return Searching::BinarySearch{}.run({ arr1k, 500 }); };
  BENCHMARK("BinarySearch    1k  end") { return Searching::BinarySearch{}.run({ arr1k, 999 }); };
  BENCHMARK("LinearSearch    1k  mid") { return Searching::LinearSearch{}.run({ arr1k, 500 }); };
  BENCHMARK("LinearSearch    1k  end") { return Searching::LinearSearch{}.run({ arr1k, 999 }); };
  BENCHMARK("Interpolation   1k  mid") { return Searching::InterpolationSearch{}.run({ arr1k, 500 }); };

  BENCHMARK("BinarySearch   10k  mid") { return Searching::BinarySearch{}.run({ arr10k, 5000 }); };
  BENCHMARK("LinearSearch   10k  mid") { return Searching::LinearSearch{}.run({ arr10k, 5000 }); };
  BENCHMARK("Interpolation  10k  mid") { return Searching::InterpolationSearch{}.run({ arr10k, 5000 }); };
}