#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <numeric>
#include <random>
#include "../src/algo/heap.h"
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

// =============================================================================
//  MaxHeap
// =============================================================================
TEST_CASE("MaxHeap - basic operations", "[heap][maxheap][correctness]")
{
	Heap::MaxHeap<int> h;

	SECTION("empty heap throws on top and pop")
	{
		REQUIRE(h.empty());
		REQUIRE(h.size() == 0);
		REQUIRE_THROWS_AS(h.top(), std::out_of_range);
		REQUIRE_THROWS_AS(h.pop(), std::out_of_range);
	}

	SECTION("single element")
	{
		h.push(42);
		REQUIRE(h.size() == 1);
		REQUIRE(h.top() == 42);
		REQUIRE(h.pop() == 42);
		REQUIRE(h.empty());
	}

	SECTION("top is always the maximum after each push")
	{
		h.push(5);
		REQUIRE(h.top() == 5);
		h.push(10);
		REQUIRE(h.top() == 10);
		h.push(3);
		REQUIRE(h.top() == 10);
		h.push(99);
		REQUIRE(h.top() == 99);
		h.push(7);
		REQUIRE(h.top() == 99);
	}

	SECTION("pop returns elements in descending order")
	{
		for (int x : {3, 1, 4, 1, 5, 9, 2, 6})
			h.push(x);

		std::vector<int> result;
		while (!h.empty()) result.push_back(h.pop());

		auto expected = result;
		std::sort(expected.begin(), expected.end(), std::greater<int>{});
		REQUIRE(result == expected);
	}

	SECTION("all equal elements")
	{
		for (int i = 0; i < 10; ++i) h.push(7);
		REQUIRE(h.top() == 7);
		REQUIRE(h.size() == 10);
		while (!h.empty()) REQUIRE(h.pop() == 7);
	}

	SECTION("negative numbers")
	{
		h.push(-5);
		h.push(-1);
		h.push(-10);
		REQUIRE(h.top() == -1);
		REQUIRE(h.pop() == -1);
		REQUIRE(h.top() == -5);
	}
}

TEST_CASE("MaxHeap - heapify constructor (Floyd O(n))", "[heap][maxheap][correctness]")
{
	auto input = random_vec(500);
	Heap::MaxHeap<int> h(input);

	REQUIRE(h.size() == input.size());

	// Max of the input must be at the top
	int expected_max = *std::max_element(input.begin(), input.end());
	REQUIRE(h.top() == expected_max);

	// drain_sorted must return all elements in ascending order
	auto sorted = h.drain_sorted();
	auto expected = input;
	std::sort(expected.begin(), expected.end());
	REQUIRE(sorted == expected);
	REQUIRE(h.empty());
}

TEST_CASE("MaxHeap - drain_sorted", "[heap][maxheap][correctness]")
{
	Heap::MaxHeap<int> h;
	std::vector<int> input = {5, 2, 8, 1, 9, 3};
	for (int x : input) h.push(x);

	auto result = h.drain_sorted();
	std::sort(input.begin(), input.end());
	REQUIRE(result == input);
	REQUIRE(h.empty());
}

// =============================================================================
//  MinHeap
// =============================================================================
TEST_CASE("MinHeap - basic operations", "[heap][minheap][correctness]")
{
	Heap::MinHeap<int> h;

	SECTION("empty heap throws on top and pop")
	{
		REQUIRE(h.empty());
		REQUIRE_THROWS_AS(h.top(), std::out_of_range);
		REQUIRE_THROWS_AS(h.pop(), std::out_of_range);
	}

	SECTION("single element")
	{
		h.push(42);
		REQUIRE(h.top() == 42);
		REQUIRE(h.pop() == 42);
		REQUIRE(h.empty());
	}

	SECTION("top is always the minimum after each push")
	{
		h.push(10);
		REQUIRE(h.top() == 10);
		h.push(5);
		REQUIRE(h.top() == 5);
		h.push(8);
		REQUIRE(h.top() == 5);
		h.push(1);
		REQUIRE(h.top() == 1);
		h.push(3);
		REQUIRE(h.top() == 1);
	}

	SECTION("pop returns elements in ascending order")
	{
		for (int x : {3, 1, 4, 1, 5, 9, 2, 6})
			h.push(x);

		std::vector<int> result;
		while (!h.empty()) result.push_back(h.pop());

		auto expected = result;
		std::sort(expected.begin(), expected.end());
		REQUIRE(result == expected);
	}

	SECTION("negative numbers")
	{
		h.push(-5);
		h.push(-1);
		h.push(-10);
		REQUIRE(h.top() == -10);
		REQUIRE(h.pop() == -10);
		REQUIRE(h.top() == -5);
	}
}

TEST_CASE("MinHeap - heapify constructor", "[heap][minheap][correctness]")
{
	auto input = random_vec(500);
	Heap::MinHeap<int> h(input);

	int expected_min = *std::min_element(input.begin(), input.end());
	REQUIRE(h.top() == expected_min);

	auto sorted = h.drain_sorted();
	auto expected = input;
	std::sort(expected.begin(), expected.end());
	REQUIRE(sorted == expected);
}

// =============================================================================
//  MaxHeap vs MinHeap symmetry
// =============================================================================
TEST_CASE("MaxHeap and MinHeap contain the same elements as input", "[heap][property]")
{
	auto input = random_vec(300);

	Heap::MaxHeap<int> mxh(input);
	Heap::MinHeap<int> mnh(input);

	// Both should produce the same multiset when drained
	auto from_max = mxh.drain_sorted();
	auto from_min = mnh.drain_sorted();

	REQUIRE(from_max == from_min);

	auto expected = input;
	std::sort(expected.begin(), expected.end());
	REQUIRE(from_max == expected);
}

// =============================================================================
//  MedianHeap
// =============================================================================
TEST_CASE("MedianHeap - basic correctness", "[heap][medianheap][correctness]")
{
	Heap::MedianHeap mh;

	SECTION("empty throws")
	{
		REQUIRE(mh.empty());
		REQUIRE_THROWS_AS(mh.median(), std::out_of_range);
	}

	SECTION("single element")
	{
		mh.push(7);
		REQUIRE(mh.median() == Catch::Approx(7.0));
	}

	SECTION("two elements - median is their average")
	{
		mh.push(3);
		mh.push(7);
		REQUIRE(mh.median() == Catch::Approx(5.0));
	}

	SECTION("odd count - median is the middle element")
	{
		for (int x : {1, 3, 5}) mh.push(x);
		REQUIRE(mh.median() == Catch::Approx(3.0));
	}

	SECTION("even count - median is average of two middle elements")
	{
		for (int x : {1, 2, 3, 4}) mh.push(x);
		REQUIRE(mh.median() == Catch::Approx(2.5));
	}

	SECTION("insertion order does not affect result")
	{
		// Same 5 elements inserted in different orders
		Heap::MedianHeap mh2;
		for (int x : {5, 1, 3, 2, 4}) mh.push(x);
		for (int x : {3, 5, 1, 4, 2}) mh2.push(x);
		REQUIRE(mh.median() == Catch::Approx(mh2.median()));
	}

	SECTION("all equal elements")
	{
		for (int i = 0; i < 6; ++i) mh.push(4);
		REQUIRE(mh.median() == Catch::Approx(4.0));
	}

	SECTION("negative numbers")
	{
		for (int x : {-4, -2, -6}) mh.push(x);
		REQUIRE(mh.median() == Catch::Approx(-4.0));
	}
}

TEST_CASE("MedianHeap - running median matches sort-based answer", "[heap][medianheap][property]")
{
	auto input = random_vec(201);  // odd size so median is exact integer
	Heap::MedianHeap mh;
	for (int x : input) mh.push(x);

	// Ground truth via sorting
	auto sorted = input;
	std::sort(sorted.begin(), sorted.end());
	double expected = sorted[sorted.size() / 2];

	REQUIRE(mh.median() == Catch::Approx(expected));
}

TEST_CASE("MedianHeap - even-size median matches sort-based answer", "[heap][medianheap][property]")
{
	auto input = random_vec(200);  // even size
	Heap::MedianHeap mh;
	for (int x : input) mh.push(x);

	auto sorted = input;
	std::sort(sorted.begin(), sorted.end());
	int n = (int)sorted.size();
	double expected = (sorted[n / 2 - 1] + (double)sorted[n / 2]) / 2.0;

	REQUIRE(mh.median() == Catch::Approx(expected));
}

// =============================================================================
//  TopK algorithms
// =============================================================================
TEST_CASE("TopK - correctness", "[heap][topk][correctness]")
{
	Heap::TopKElements heap_algo;
	Heap::TopKSort     sort_algo;

	auto input = random_vec(1000);

	// Sort descending to compute expected answers
	auto sorted_desc = input;
	std::sort(sorted_desc.rbegin(), sorted_desc.rend());

	SECTION("k = 1 (maximum element)")
	{
		Heap::TopKInput in{input, 1};
		REQUIRE(heap_algo.run(in) == std::vector<int>{sorted_desc[0]});
		REQUIRE(sort_algo.run(in) == std::vector<int>{sorted_desc[0]});
	}

	SECTION("k = 10")
	{
		Heap::TopKInput in{input, 10};
		auto expected = std::vector<int>(sorted_desc.begin(), sorted_desc.begin() + 10);
		REQUIRE(heap_algo.run(in) == expected);
		REQUIRE(sort_algo.run(in) == expected);
	}

	SECTION("k = n (all elements)")
	{
		Heap::TopKInput in{input, (int)input.size()};
		REQUIRE(heap_algo.run(in) == sorted_desc);
		REQUIRE(sort_algo.run(in) == sorted_desc);
	}

	SECTION("heap result matches sort result for any k")
	{
		for (int k : {1, 5, 50, 200, 500})
		{
			Heap::TopKInput in{input, k};
			REQUIRE(heap_algo.run(in) == sort_algo.run(in));
		}
	}
}

TEST_CASE("TopK - edge cases", "[heap][topk][correctness]")
{
	Heap::TopKElements algo;

	SECTION("all equal elements")
	{
		std::vector<int> input(20, 5);
		auto result = algo.run({input, 5});
		REQUIRE(result == std::vector<int>(5, 5));
	}

	SECTION("already sorted input")
	{
		auto input = sorted_vec(100);
		auto result = algo.run({input, 3});
		REQUIRE(result == std::vector<int>({99, 98, 97}));
	}

	SECTION("reverse sorted input")
	{
		auto input = reversed_vec(100);  // 99..0
		auto result = algo.run({input, 3});
		REQUIRE(result == std::vector<int>({99, 98, 97}));
	}
}

// =============================================================================
//  KthLargest algorithms
// =============================================================================
TEST_CASE("KthLargest - correctness", "[heap][kthlargest][correctness]")
{
	Heap::KthLargestHeap heap_algo;
	Heap::KthLargestNth  nth_algo;

	auto input = random_vec(1000);
	auto sorted_desc = input;
	std::sort(sorted_desc.rbegin(), sorted_desc.rend());

	SECTION("k=1 is the maximum")
	{
		Heap::KthInput in{input, 1};
		REQUIRE(heap_algo.run(in) == sorted_desc[0]);
		REQUIRE(nth_algo.run(in)  == sorted_desc[0]);
	}

	SECTION("k=n is the minimum")
	{
		Heap::KthInput in{input, (int)input.size()};
		REQUIRE(heap_algo.run(in) == sorted_desc.back());
		REQUIRE(nth_algo.run(in)  == sorted_desc.back());
	}

	SECTION("k=2")
	{
		Heap::KthInput in{input, 2};
		REQUIRE(heap_algo.run(in) == sorted_desc[1]);
		REQUIRE(nth_algo.run(in)  == sorted_desc[1]);
	}

	SECTION("heap and nth_element agree for several k values")
	{
		for (int k : {1, 10, 100, 500, 999, 1000})
		{
			Heap::KthInput in{input, k};
			REQUIRE(heap_algo.run(in) == nth_algo.run(in));
		}
	}
}

TEST_CASE("KthLargest - edge cases", "[heap][kthlargest][correctness]")
{
	Heap::KthLargestHeap algo;

	SECTION("single element, k=1")
	{
		REQUIRE(algo.run({{42}, 1}) == 42);
	}

	SECTION("all equal elements")
	{
		std::vector<int> input(10, 7);
		REQUIRE(algo.run({input, 3}) == 7);
	}

	SECTION("two elements")
	{
		REQUIRE(algo.run({{3, 9}, 1}) == 9);
		REQUIRE(algo.run({{3, 9}, 2}) == 3);
	}
}

// =============================================================================
//  MedianFinder algorithms
// =============================================================================
TEST_CASE("MedianFinder - heap matches sort baseline", "[heap][medianfinder][correctness]")
{
	Heap::MedianFinderHeap heap_algo;
	Heap::MedianFinderSort sort_algo;

	SECTION("odd-size random array")
	{
		auto input = random_vec(201);
		REQUIRE(heap_algo.run(input) == Catch::Approx(sort_algo.run(input)));
	}

	SECTION("even-size random array")
	{
		auto input = random_vec(200);
		REQUIRE(heap_algo.run(input) == Catch::Approx(sort_algo.run(input)));
	}

	SECTION("single element")
	{
		REQUIRE(heap_algo.run({42}) == Catch::Approx(42.0));
	}

	SECTION("two elements")
	{
		REQUIRE(heap_algo.run({1, 3}) == Catch::Approx(2.0));
	}

	SECTION("all equal")
	{
		std::vector<int> input(50, 9);
		REQUIRE(heap_algo.run(input) == Catch::Approx(9.0));
		REQUIRE(sort_algo.run(input) == Catch::Approx(9.0));
	}

	SECTION("sorted input")
	{
		auto input = sorted_vec(99);  // 0..98, median = 49
		REQUIRE(heap_algo.run(input) == Catch::Approx(sort_algo.run(input)));
	}

	SECTION("reversed input")
	{
		auto input = reversed_vec(100);
		REQUIRE(heap_algo.run(input) == Catch::Approx(sort_algo.run(input)));
	}
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("Heap - data structure benchmarks", "[heap][benchmark][!benchmark]")
{
	auto r1k  = random_vec(1000);
	auto r10k = random_vec(10000);

	// --- MaxHeap: build via push-one-by-one vs heapify constructor ---------------
	BENCHMARK("MaxHeap  pushX1k (one-by-one)")
	{
		Heap::MaxHeap<int> h;
		for (int x : r1k) h.push(x);
		return h.top();
	};
	BENCHMARK("MaxHeap  heapify 1k (Floyd O(n))")
	{
		Heap::MaxHeap<int> h(r1k);
		return h.top();
	};
	BENCHMARK("MinHeap  pushX1k (one-by-one)")
	{
		Heap::MinHeap<int> h;
		for (int x : r1k) h.push(x);
		return h.top();
	};
	BENCHMARK("MinHeap  heapify 1k (Floyd O(n))")
	{
		Heap::MinHeap<int> h(r1k);
		return h.top();
	};

	// --- TopK: heap O(n log k) vs full sort O(n log n) ---------------------------
	// When k is tiny the heap dominates; as k→n they converge.
	BENCHMARK("TopK heap  n=10k k=10")
	{
		return Heap::TopKElements{}.run({r10k, 10});
	};
	BENCHMARK("TopK sort  n=10k k=10")
	{
		return Heap::TopKSort{}.run({r10k, 10});
	};
	BENCHMARK("TopK heap  n=10k k=100")
	{
		return Heap::TopKElements{}.run({r10k, 100});
	};
	BENCHMARK("TopK sort  n=10k k=100")
	{
		return Heap::TopKSort{}.run({r10k, 100});
	};
	BENCHMARK("TopK heap  n=10k k=5000")
	{
		return Heap::TopKElements{}.run({r10k, 5000});
	};
	BENCHMARK("TopK sort  n=10k k=5000")
	{
		return Heap::TopKSort{}.run({r10k, 5000});
	};

	// --- KthLargest: heap O(n log k) vs nth_element O(n) -------------------------
	BENCHMARK("KthLargest heap  n=10k k=10")
	{
		return Heap::KthLargestHeap{}.run({r10k, 10});
	};
	BENCHMARK("KthLargest nth   n=10k k=10")
	{
		return Heap::KthLargestNth{}.run({r10k, 10});
	};
	BENCHMARK("KthLargest heap  n=10k k=5000")
	{
		return Heap::KthLargestHeap{}.run({r10k, 5000});
	};
	BENCHMARK("KthLargest nth   n=10k k=5000")
	{
		return Heap::KthLargestNth{}.run({r10k, 5000});
	};

	// --- MedianFinder: two-heap vs sort ------------------------------------------
	BENCHMARK("MedianFinder heap  n=1k")
	{
		return Heap::MedianFinderHeap{}.run(r1k);
	};
	BENCHMARK("MedianFinder sort  n=1k")
	{
		return Heap::MedianFinderSort{}.run(r1k);
	};
	BENCHMARK("MedianFinder heap  n=10k")
	{
		return Heap::MedianFinderHeap{}.run(r10k);
	};
	BENCHMARK("MedianFinder sort  n=10k")
	{
		return Heap::MedianFinderSort{}.run(r10k);
	};
}
