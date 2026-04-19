#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <numeric>
#include <random>
#include "../src/algo/linked_list.h"
#include "../src/dsa_framework/framework.h"

// --- helpers -----------------------------------------------------------------
static std::vector<int> iota_vec(int n)
{
	std::vector<int> v(n);
	std::iota(v.begin(), v.end(), 0);
	return v;
}
static std::vector<int> random_vec(int n, unsigned seed = 42)
{
	std::mt19937 rng(seed);
	std::uniform_int_distribution<int> d(0, 100000);
	std::vector<int> v(n);
	std::generate(v.begin(), v.end(), [&] { return d(rng); });
	return v;
}

// =============================================================================
//  SinglyLinkedList - basic operations
// =============================================================================
TEST_CASE("SinglyLinkedList - push and pop", "[linkedlist][correctness]")
{
	DSALog::info("[linkedlist] push and pop");
	LinkedList::SinglyLinkedList<int> l;

	SECTION("empty list")
	{
		REQUIRE(l.empty());
		REQUIRE(l.size() == 0);
		REQUIRE(l.head() == nullptr);
		REQUIRE_THROWS_AS(l.pop_front(), std::out_of_range);
	}

	SECTION("push_back maintains order")
	{
		l.push_back(1); l.push_back(2); l.push_back(3);
		REQUIRE(l.to_vector() == std::vector<int>{1, 2, 3});
		REQUIRE(l.size() == 3);
	}

	SECTION("push_front reverses insertion order")
	{
		l.push_front(1); l.push_front(2); l.push_front(3);
		REQUIRE(l.to_vector() == std::vector<int>{3, 2, 1});
	}

	SECTION("pop_front removes from the front")
	{
		l.push_back(10); l.push_back(20); l.push_back(30);
		REQUIRE(l.pop_front() == 10);
		REQUIRE(l.pop_front() == 20);
		REQUIRE(l.size() == 1);
		REQUIRE(l.pop_front() == 30);
		REQUIRE(l.empty());
	}

	SECTION("single element push and pop")
	{
		l.push_back(42);
		REQUIRE(l.size() == 1);
		REQUIRE(l.pop_front() == 42);
		REQUIRE(l.empty());
	}
}

// =============================================================================
//  SinglyLinkedList - reverse
// =============================================================================
TEST_CASE("SinglyLinkedList - reverse", "[linkedlist][correctness]")
{
	DSALog::info("[linkedlist] reverse");

	SECTION("reverse empty list is safe")
	{
		LinkedList::SinglyLinkedList<int> l;
		l.reverse();
		REQUIRE(l.empty());
	}

	SECTION("reverse single element is identity")
	{
		LinkedList::SinglyLinkedList<int> l;
		l.push_back(7);
		l.reverse();
		REQUIRE(l.to_vector() == std::vector<int>{7});
	}

	SECTION("reverse two elements")
	{
		LinkedList::SinglyLinkedList<int> l;
		l.push_back(1); l.push_back(2);
		l.reverse();
		REQUIRE(l.to_vector() == std::vector<int>{2, 1});
	}

	SECTION("reverse odd-length list")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{1, 2, 3, 4, 5});
		l.reverse();
		REQUIRE(l.to_vector() == std::vector<int>{5, 4, 3, 2, 1});
	}

	SECTION("reverse even-length list")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{1, 2, 3, 4});
		l.reverse();
		REQUIRE(l.to_vector() == std::vector<int>{4, 3, 2, 1});
	}

	SECTION("double reverse is identity")
	{
		auto input = std::vector<int>{3, 1, 4, 1, 5, 9};
		LinkedList::SinglyLinkedList<int> l(input);
		l.reverse();
		l.reverse();
		REQUIRE(l.to_vector() == input);
	}
}

// =============================================================================
//  SinglyLinkedList - has_cycle
// =============================================================================
TEST_CASE("SinglyLinkedList - has_cycle (Floyd's algorithm)", "[linkedlist][correctness]")
{
	DSALog::info("[linkedlist] has_cycle Floyd");

	SECTION("empty list has no cycle")
	{
		LinkedList::SinglyLinkedList<int> l;
		REQUIRE_FALSE(l.has_cycle());
	}

	SECTION("normal list has no cycle")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{1, 2, 3, 4, 5});
		REQUIRE_FALSE(l.has_cycle());
	}

	SECTION("manually created cycle is detected")
	{
		// Build 1 -> 2 -> 3 -> 4 -> (back to 2)
		auto* n1 = new LinkedList::Node<int>(1);
		auto* n2 = new LinkedList::Node<int>(2);
		auto* n3 = new LinkedList::Node<int>(3);
		auto* n4 = new LinkedList::Node<int>(4);
		n1->next = n2; n2->next = n3; n3->next = n4; n4->next = n2;  // cycle

		// Use the algorithm directly on raw nodes (can't use SinglyLinkedList — it
		// would try to delete nodes in the cycle and loop forever)
		LinkedList::Node<int>* slow = n1;
		LinkedList::Node<int>* fast = n1;
		bool cycle_found = false;
		while (fast && fast->next)
		{
			slow = slow->next;
			fast = fast->next->next;
			if (slow == fast) { cycle_found = true; break; }
		}
		REQUIRE(cycle_found);

		// Break cycle before cleanup to avoid infinite loop in destructor
		n4->next = nullptr;
		delete n1; delete n2; delete n3; delete n4;
	}
}

// =============================================================================
//  SinglyLinkedList - find_middle
// =============================================================================
TEST_CASE("SinglyLinkedList - find_middle", "[linkedlist][correctness]")
{
	DSALog::info("[linkedlist] find_middle");

	SECTION("single element")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{42});
		REQUIRE(l.find_middle()->val == 42);
	}

	SECTION("odd length - exact middle")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{1, 2, 3, 4, 5});
		REQUIRE(l.find_middle()->val == 3);
	}

	SECTION("even length - lower middle")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{1, 2, 3, 4});
		REQUIRE(l.find_middle()->val == 2);
	}

	SECTION("two elements - first element")
	{
		LinkedList::SinglyLinkedList<int> l(std::vector<int>{10, 20});
		REQUIRE(l.find_middle()->val == 10);
	}
}

// =============================================================================
//  ListReverse vs ArrayReverse
// =============================================================================
TEST_CASE("ListReverse correctness", "[linkedlist][correctness]")
{
	DSALog::info("[linkedlist] ListReverse vs ArrayReverse");
	LinkedList::ListReverse  list_algo;
	LinkedList::ArrayReverse arr_algo;

	SECTION("empty")
	{
		REQUIRE(list_algo.run({}).empty());
		REQUIRE(arr_algo.run({}).empty());
	}

	SECTION("single element")
	{
		REQUIRE(list_algo.run({7}) == std::vector<int>{7});
	}

	SECTION("matches std::reverse")
	{
		auto input = random_vec(500);
		REQUIRE(list_algo.run(input) == arr_algo.run(input));
	}

	SECTION("reverse of sorted is reversed-sorted")
	{
		auto input    = iota_vec(100);
		auto result   = list_algo.run(input);
		auto expected = input;
		std::reverse(expected.begin(), expected.end());
		REQUIRE(result == expected);
	}
}

// =============================================================================
//  ListMergeSorted vs ArrayMergeSorted
// =============================================================================
TEST_CASE("MergeSorted correctness", "[linkedlist][correctness]")
{
	DSALog::info("[linkedlist] MergeSorted");
	LinkedList::ListMergeSorted  list_algo;
	LinkedList::ArrayMergeSorted arr_algo;

	SECTION("both empty")
	{
		REQUIRE(list_algo.run({{}, {}}).empty());
	}

	SECTION("one side empty")
	{
		LinkedList::MergeInput in{{1, 2, 3}, {}};
		REQUIRE(list_algo.run(in) == std::vector<int>{1, 2, 3});
		REQUIRE(arr_algo.run(in)  == std::vector<int>{1, 2, 3});
	}

	SECTION("basic merge")
	{
		LinkedList::MergeInput in{{1, 3, 5}, {2, 4, 6}};
		auto expected = std::vector<int>{1, 2, 3, 4, 5, 6};
		REQUIRE(list_algo.run(in) == expected);
		REQUIRE(arr_algo.run(in)  == expected);
	}

	SECTION("equal elements interleaved")
	{
		LinkedList::MergeInput in{{1, 1, 3}, {1, 2, 3}};
		auto expected = std::vector<int>{1, 1, 1, 2, 3, 3};
		REQUIRE(list_algo.run(in) == expected);
		REQUIRE(arr_algo.run(in)  == expected);
	}

	SECTION("different lengths")
	{
		auto a = iota_vec(10);
		auto b = std::vector<int>{2, 5, 8};
		LinkedList::MergeInput in{a, b};
		REQUIRE(list_algo.run(in) == arr_algo.run(in));
	}

	SECTION("list and array results always match on random input")
	{
		auto a = random_vec(200, 1); std::sort(a.begin(), a.end());
		auto b = random_vec(200, 2); std::sort(b.begin(), b.end());
		LinkedList::MergeInput in{a, b};
		REQUIRE(list_algo.run(in) == arr_algo.run(in));
	}
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("LinkedList benchmarks", "[linkedlist][benchmark][!benchmark]")
{
	auto r1k  = random_vec(1000);
	auto r10k = random_vec(10000);

	auto s10k = r10k; std::sort(s10k.begin(), s10k.end());
	auto half1 = std::vector<int>(s10k.begin(), s10k.begin() + 5000);
	auto half2 = std::vector<int>(s10k.begin() + 5000, s10k.end());

	BENCHMARK("ListReverse  1k")  { return LinkedList::ListReverse{}.run(r1k); };
	BENCHMARK("ArrayReverse 1k")  { return LinkedList::ArrayReverse{}.run(r1k); };
	BENCHMARK("ListReverse  10k") { return LinkedList::ListReverse{}.run(r10k); };
	BENCHMARK("ArrayReverse 10k") { return LinkedList::ArrayReverse{}.run(r10k); };

	BENCHMARK("ListMerge   10k+10k") { return LinkedList::ListMergeSorted{}.run({half1, half2}); };
	BENCHMARK("ArrayMerge  10k+10k") { return LinkedList::ArrayMergeSorted{}.run({half1, half2}); };
}
