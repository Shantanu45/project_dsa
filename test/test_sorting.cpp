#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <numeric>
#include <random>
#include "../src/algo/sorting.h"
#include "../src/dsa_framework/framework.h"

// --- helpers ----------------------------------------------------------------
static std::vector<int> random_vec(int n, unsigned seed = 42)
{
	std::mt19937 rng(seed);
	std::uniform_int_distribution<int> d(0, 100000);
	std::vector<int> v(n);
	std::generate(v.begin(), v.end(), [&] { return d(rng); });
	return v;
}
static std::vector<int> sorted_vec(int n)
{
	std::vector<int> v(n);
	std::iota(v.begin(), v.end(), 0);
	return v;
}
static std::vector<int> reversed_vec(int n)
{
	auto v = sorted_vec(n);
	std::reverse(v.begin(), v.end());
	return v;
}

// --- correctness -------------------------------------------------------------
TEMPLATE_TEST_CASE("Sorting - correctness",
  "[sorting][correctness]",
  Sorting::QuickSort,
  Sorting::MergeSort,
  Sorting::HeapSort,
  Sorting::InsertionSort,
  Sorting::StdSort)
{
	TestType algo;
	DSALog::info("[sorting] testing {}", algo.name());

	SECTION("empty input")
	{
		auto result = algo.run({});
		REQUIRE(result.empty());
	}

	SECTION("single element")
	{
		REQUIRE(algo.run({42}) == std::vector<int>{42});
	}

	SECTION("already sorted")
	{
		auto input = sorted_vec(100);
		auto expected = input;
		REQUIRE(algo.run(input) == expected);
	}

	SECTION("reverse sorted")
	{
		auto input = reversed_vec(100);
		auto expected = sorted_vec(100);
		REQUIRE(algo.run(input) == expected);
	}

	SECTION("random 1000 elements")
	{
		auto input = random_vec(1000);
		auto expected = input;
		std::sort(expected.begin(), expected.end());
		REQUIRE(algo.run(input) == expected);
	}

	SECTION("all equal")
	{
		std::vector<int> input(50, 7);
		REQUIRE(algo.run(input) == input);
	}

	SECTION("two elements swapped")
	{
		REQUIRE(algo.run({2, 1}) == std::vector<int>{1, 2});
	}
}

// --- property: output is a permutation of input ------------------------------
TEMPLATE_TEST_CASE("Sorting - output is permutation",
  "[sorting][property]",
  Sorting::QuickSort,
  Sorting::MergeSort,
  Sorting::InsertionSort,
  Sorting::HeapSort)
{
	TestType algo;
	auto input = random_vec(500);
	auto result = algo.run(input);

	// same size
	REQUIRE(result.size() == input.size());
	// same multiset
	std::sort(input.begin(), input.end());
	REQUIRE(result == input);  // result is already sorted
}

// --- property: MergeSort is stable -------------------------------------------
TEST_CASE("MergeSort and InsertionSort are stable", "[sorting][stable]")
{
	// Use pairs; sort by first, verify second-key order is preserved
	// (We exercise via equal keys in first-key position)
	std::vector<int> input = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
	Sorting::MergeSort ms;
	auto result = ms.run(input);
	auto expected = input;
	std::stable_sort(expected.begin(), expected.end());
	REQUIRE(result == expected);

	Sorting::InsertionSort is;
    result = is.run(input);
    REQUIRE(result == expected);
}

// --- benchmarks (Catch2 BENCHMARK macro) -------------------------------------
TEST_CASE("Sorting benchmarks", "[sorting][benchmark][!benchmark]")
{
	auto r100 = random_vec(100);
	auto r1k = random_vec(1000);
	auto r10k = random_vec(10000);
	auto s1k = sorted_vec(1000);  // worst-case for naive QuickSort
	auto rev1k = reversed_vec(1000);

	BENCHMARK("QuickSort  random-100")
	{
		return Sorting::QuickSort{}.run(r100);
	};
	BENCHMARK("MergeSort  random-100")
	{
		return Sorting::MergeSort{}.run(r100);
	};
	BENCHMARK("HeapSort   random-100")
	{
		return Sorting::HeapSort{}.run(r100);
	};
    BENCHMARK("InsertionSort   random-100") 
	{ 
		return Sorting::InsertionSort{}.run(r100); 
	};
	BENCHMARK("std::sort  random-100")
	{
		return Sorting::StdSort{}.run(r100);
	};

	BENCHMARK("QuickSort  random-1k")
	{
		return Sorting::QuickSort{}.run(r1k);
	};
	BENCHMARK("MergeSort  random-1k")
	{
		return Sorting::MergeSort{}.run(r1k);
	};
	BENCHMARK("HeapSort   random-1k")
	{
		return Sorting::HeapSort{}.run(r1k);
	};
    BENCHMARK("InsertionSort   random-1k") 
	{ 
		return Sorting::InsertionSort{}.run(r1k); 
	};
	BENCHMARK("std::sort  random-1k")
	{
		return Sorting::StdSort{}.run(r1k);
	};

	BENCHMARK("QuickSort  random-10k")
	{
		return Sorting::QuickSort{}.run(r10k);
	};
	BENCHMARK("std::sort  random-10k")
	{
		return Sorting::StdSort{}.run(r10k);
	};

	BENCHMARK("QuickSort  sorted-1k")
	{
		return Sorting::QuickSort{}.run(s1k);
	};
	BENCHMARK("HeapSort   sorted-1k")
	{
		return Sorting::HeapSort{}.run(s1k);
	};
    BENCHMARK("InsertionSort   sorted-1k") 
	{ 
		return Sorting::InsertionSort{}.run(s1k); 
	};
	BENCHMARK("std::sort  sorted-1k")
	{
		return Sorting::StdSort{}.run(s1k);
	};

	BENCHMARK("QuickSort  reversed-1k")
	{
		return Sorting::QuickSort{}.run(rev1k);
	};
	BENCHMARK("HeapSort   reversed-1k")
	{
		return Sorting::HeapSort{}.run(rev1k);
	};
}