#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <numeric>
#include <random>
#include "../src/algo/tree.h"
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
static std::vector<int> sorted_vec(int n) { std::vector<int> v(n); std::iota(v.begin(), v.end(), 0); return v; }
static std::vector<int> reversed_vec(int n) { auto v = sorted_vec(n); std::reverse(v.begin(), v.end()); return v; }

// =============================================================================
//  BST
// =============================================================================
TEST_CASE("BST - insert and search", "[tree][bst][correctness]")
{
	DSALog::info("[tree] BST insert and search");
	Tree::BST t;

	SECTION("empty tree")
	{
		REQUIRE(t.empty());
		REQUIRE(t.size() == 0);
		REQUIRE(t.height() == 0);
		REQUIRE_FALSE(t.search(42));
	}

	SECTION("insert and find single element")
	{
		t.insert(10);
		REQUIRE(t.search(10));
		REQUIRE_FALSE(t.search(9));
		REQUIRE(t.size() == 1);
	}

	SECTION("BST ordering property - inorder is sorted")
	{
		for (int x : {5, 3, 7, 1, 4, 6, 8}) t.insert(x);
		auto result = t.inorder();
		auto sorted = result;
		std::sort(sorted.begin(), sorted.end());
		REQUIRE(result == sorted);
	}

	SECTION("duplicate elements handled (go right)")
	{
		t.insert(5); t.insert(5); t.insert(5);
		REQUIRE(t.size() == 3);
		auto v = t.inorder();
		REQUIRE(v == std::vector<int>{5, 5, 5});
	}

	SECTION("insert ascending (worst case for plain BST)")
	{
		for (int i = 0; i < 10; ++i) t.insert(i);
		REQUIRE(t.size() == 10);
		REQUIRE(t.height() == 10);  // degenerates to linked list
		REQUIRE(t.inorder() == sorted_vec(10));
	}
}

TEST_CASE("BST - remove", "[tree][bst][correctness]")
{
	DSALog::info("[tree] BST remove");
	Tree::BST t;
	for (int x : {5, 3, 7, 1, 4, 6, 8}) t.insert(x);

	SECTION("remove leaf node")
	{
		t.remove(1);
		REQUIRE_FALSE(t.search(1));
		REQUIRE(t.size() == 6);
		auto v = t.inorder();
		REQUIRE(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("remove node with one child")
	{
		t.remove(3);  // 3 has children 1 and 4
		REQUIRE_FALSE(t.search(3));
		REQUIRE(t.search(1));
		REQUIRE(t.search(4));
	}

	SECTION("remove node with two children (successor replacement)")
	{
		t.remove(5);  // root — replaced by in-order successor (6)
		REQUIRE_FALSE(t.search(5));
		REQUIRE(t.size() == 6);
		auto v = t.inorder();
		REQUIRE(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("remove non-existent element is a no-op")
	{
		int before = t.size();
		t.remove(99);
		REQUIRE(t.size() == before);
	}

	SECTION("remove all elements one by one")
	{
		for (int x : {5, 3, 7, 1, 4, 6, 8}) t.remove(x);
		REQUIRE(t.empty());
	}
}

// =============================================================================
//  AVL
// =============================================================================
TEST_CASE("AVL - insert and balance", "[tree][avl][correctness]")
{
	DSALog::info("[tree] AVL insert and balance");
	Tree::AVL t;

	SECTION("empty tree")
	{
		REQUIRE(t.empty());
		REQUIRE(t.height() == 0);
	}

	SECTION("inorder is always sorted")
	{
		for (int x : {5, 3, 7, 1, 4, 6, 8}) t.insert(x);
		auto v = t.inorder();
		REQUIRE(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("height stays O(log n) on sorted input (BST killer case)")
	{
		// Inserting 0,1,...,99 in order makes BST height=100 but AVL height≈10
		for (int i = 0; i < 100; ++i) t.insert(i);
		REQUIRE(t.height() <= 14);  // 1.44 * log2(101) ≈ 9.6, allow slack
		REQUIRE(t.inorder() == sorted_vec(100));
	}

	SECTION("height stays O(log n) on reverse-sorted input")
	{
		for (int i = 99; i >= 0; --i) t.insert(i);
		REQUIRE(t.height() <= 14);
		REQUIRE(t.inorder() == sorted_vec(100));
	}

	SECTION("duplicate elements")
	{
		t.insert(5); t.insert(5); t.insert(5);
		REQUIRE(t.size() == 3);
		REQUIRE(t.inorder() == std::vector<int>{5, 5, 5});
	}
}

TEST_CASE("AVL - remove", "[tree][avl][correctness]")
{
	DSALog::info("[tree] AVL remove");
	Tree::AVL t;
	for (int x : {5, 3, 7, 1, 4, 6, 8}) t.insert(x);

	SECTION("remove leaf")
	{
		t.remove(1);
		REQUIRE_FALSE(t.search(1));
		auto v = t.inorder();
		REQUIRE(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("remove root")
	{
		t.remove(5);
		REQUIRE_FALSE(t.search(5));
		REQUIRE(t.size() == 6);
		auto v = t.inorder();
		REQUIRE(std::is_sorted(v.begin(), v.end()));
	}

	SECTION("tree remains balanced after many removals")
	{
		for (int i = 1; i <= 100; ++i) t.insert(i);
		for (int i = 1; i <= 50; ++i)  t.remove(i);
		REQUIRE(t.height() <= 14);
		auto v = t.inorder();
		REQUIRE(std::is_sorted(v.begin(), v.end()));
	}
}

// =============================================================================
//  BST vs AVL comparison
// =============================================================================
TEST_CASE("BST vs AVL - height on sorted input", "[tree][correctness]")
{
	DSALog::info("[tree] BST vs AVL height comparison");
	Tree::BST bst;
	Tree::AVL avl;
	int n = 200;
	for (int i = 0; i < n; ++i) { bst.insert(i); avl.insert(i); }

	REQUIRE(bst.height() == n);           // worst case: linear chain
	REQUIRE(avl.height() <= 14);          // O(log n) guaranteed
	REQUIRE(bst.inorder() == avl.inorder());
}

TEST_CASE("BST and AVL inorder always matches std::sort", "[tree][property]")
{
	DSALog::info("[tree] BST and AVL inorder property");
	auto input = random_vec(300);
	auto expected = input;
	std::sort(expected.begin(), expected.end());

	Tree::BST bst;
	Tree::AVL avl;
	for (int v : input) { bst.insert(v); avl.insert(v); }

	REQUIRE(bst.inorder() == expected);
	REQUIRE(avl.inorder() == expected);
}

// =============================================================================
//  TreeSearch algorithm wrappers
// =============================================================================
TEST_CASE("TreeSearch - all three agree", "[tree][correctness]")
{
	DSALog::info("[tree] TreeSearch BST vs AVL vs std::set");
	Tree::BSTSearch    bst_algo;
	Tree::AVLSearch    avl_algo;
	Tree::StdSetSearch set_algo;

	auto vals = random_vec(100);

	SECTION("target present")
	{
		Tree::TreeSearchInput in{vals, vals[50]};
		REQUIRE(bst_algo.run(in) == true);
		REQUIRE(avl_algo.run(in) == true);
		REQUIRE(set_algo.run(in) == true);
	}

	SECTION("target absent")
	{
		Tree::TreeSearchInput in{vals, -1};
		REQUIRE(bst_algo.run(in) == false);
		REQUIRE(avl_algo.run(in) == false);
		REQUIRE(set_algo.run(in) == false);
	}

	SECTION("sorted input - BST degenerates but still correct")
	{
		auto sorted = sorted_vec(100);
		Tree::TreeSearchInput in{sorted, 99};
		REQUIRE(bst_algo.run(in) == true);
		REQUIRE(avl_algo.run(in) == true);
	}
}

// =============================================================================
//  TreeSort algorithm wrappers
// =============================================================================
TEST_CASE("BSTSort and AVLSort output matches std::sort", "[tree][correctness]")
{
	DSALog::info("[tree] BSTSort and AVLSort");
	Tree::BSTSort bst_algo;
	Tree::AVLSort avl_algo;

	SECTION("random input")
	{
		auto input = random_vec(200);
		auto expected = input;
		std::sort(expected.begin(), expected.end());
		REQUIRE(bst_algo.run(input) == expected);
		REQUIRE(avl_algo.run(input) == expected);
	}

	SECTION("already sorted")
	{
		auto input = sorted_vec(100);
		REQUIRE(bst_algo.run(input) == input);
		REQUIRE(avl_algo.run(input) == input);
	}

	SECTION("reverse sorted")
	{
		auto input    = reversed_vec(100);
		auto expected = sorted_vec(100);
		REQUIRE(bst_algo.run(input) == expected);
		REQUIRE(avl_algo.run(input) == expected);
	}

	SECTION("all equal")
	{
		std::vector<int> input(20, 5);
		REQUIRE(bst_algo.run(input) == input);
		REQUIRE(avl_algo.run(input) == input);
	}
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("Tree benchmarks", "[tree][benchmark][!benchmark]")
{
	auto r1k     = random_vec(1000);
	auto r10k    = random_vec(10000);
	auto s1k     = sorted_vec(1000);
	auto s10k    = sorted_vec(10000);
	auto rev1k   = reversed_vec(1000);

	BENCHMARK("BSTSort  random-1k")  { return Tree::BSTSort{}.run(r1k); };
	BENCHMARK("AVLSort  random-1k")  { return Tree::AVLSort{}.run(r1k); };
	BENCHMARK("BSTSort  sorted-1k")  { return Tree::BSTSort{}.run(s1k); };
	BENCHMARK("AVLSort  sorted-1k")  { return Tree::AVLSort{}.run(s1k); };

	BENCHMARK("BSTSearch random-1k")  { return Tree::BSTSearch{}.run({r1k, r1k[500]}); };
	BENCHMARK("AVLSearch random-1k")  { return Tree::AVLSearch{}.run({r1k, r1k[500]}); };
	BENCHMARK("std::set  random-1k")  { return Tree::StdSetSearch{}.run({r1k, r1k[500]}); };

	BENCHMARK("BSTSearch sorted-1k")  { return Tree::BSTSearch{}.run({s1k, 999}); };
	BENCHMARK("AVLSearch sorted-1k")  { return Tree::AVLSearch{}.run({s1k, 999}); };

	BENCHMARK("BSTSort  random-10k") { return Tree::BSTSort{}.run(r10k); };
	BENCHMARK("AVLSort  random-10k") { return Tree::AVLSort{}.run(r10k); };
}
