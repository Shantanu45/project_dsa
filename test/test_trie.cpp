#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <string>
#include <vector>
#include "../src/algo/trie.h"
#include "../src/dsa_framework/framework.h"

// --- helpers -----------------------------------------------------------------
// Generates n words of the form "word0", "word1", ..., "word(n-1)"
static std::vector<std::string> numbered_words(int n, const std::string& prefix = "word")
{
	std::vector<std::string> v;
	v.reserve(n);
	for (int i = 0; i < n; ++i) v.push_back(prefix + std::to_string(i));
	return v;
}

// =============================================================================
//  Trie data structure
// =============================================================================
TEST_CASE("Trie - insert and search", "[trie][correctness]")
{
	DSALog::info("[trie] insert and search");
	Trie::Trie t;

	SECTION("empty trie finds nothing")
	{
		REQUIRE_FALSE(t.search("hello"));
		REQUIRE_FALSE(t.starts_with("he"));
		REQUIRE(t.empty());
	}

	SECTION("search only returns true for complete words")
	{
		t.insert("hello");
		REQUIRE(t.search("hello"));
		REQUIRE_FALSE(t.search("hell"));   // prefix, not a word
		REQUIRE_FALSE(t.search("helloo")); // word + extra char
	}

	SECTION("starts_with returns true for any prefix")
	{
		t.insert("hello");
		REQUIRE(t.starts_with("h"));
		REQUIRE(t.starts_with("he"));
		REQUIRE(t.starts_with("hell"));
		REQUIRE(t.starts_with("hello"));
		REQUIRE_FALSE(t.starts_with("world"));
	}

	SECTION("inserting a prefix of an existing word")
	{
		t.insert("hello");
		t.insert("hell");
		REQUIRE(t.search("hell"));
		REQUIRE(t.search("hello"));
		REQUIRE(t.size() == 2);
	}

	SECTION("inserting duplicate is a no-op")
	{
		t.insert("abc");
		t.insert("abc");
		REQUIRE(t.size() == 1);
	}

	SECTION("multiple words with shared prefix")
	{
		for (const std::string& w : {"car", "card", "care", "cat", "dog"})
			t.insert(w);
		REQUIRE(t.size() == 5);
		REQUIRE(t.search("car"));
		REQUIRE(t.search("card"));
		REQUIRE(t.search("care"));
		REQUIRE(t.search("cat"));
		REQUIRE(t.search("dog"));
		REQUIRE_FALSE(t.search("ca"));
		REQUIRE_FALSE(t.search("do"));
	}

	SECTION("single character words")
	{
		t.insert("a");
		t.insert("b");
		REQUIRE(t.search("a"));
		REQUIRE(t.search("b"));
		REQUIRE_FALSE(t.search("ab"));
	}
}

TEST_CASE("Trie - words_with_prefix", "[trie][correctness]")
{
	DSALog::info("[trie] words_with_prefix");
	Trie::Trie t;
	for (const std::string& w : {"car", "card", "care", "careful", "cat", "dog", "door"})
		t.insert(w);

	SECTION("prefix with multiple matches")
	{
		auto results = t.words_with_prefix("car");
		REQUIRE(results.size() == 4);
		std::sort(results.begin(), results.end());
		REQUIRE(results == std::vector<std::string>{"car", "card", "care", "careful"});
	}

	SECTION("prefix that is an exact word")
	{
		auto results = t.words_with_prefix("dog");
		REQUIRE(results.size() == 1);
		REQUIRE(results[0] == "dog");
	}

	SECTION("prefix with no matches")
	{
		REQUIRE(t.words_with_prefix("xyz").empty());
	}

	SECTION("empty prefix returns all words")
	{
		auto results = t.words_with_prefix("");
		REQUIRE(results.size() == t.size());
	}

	SECTION("results are in lexicographic order")
	{
		auto results = t.words_with_prefix("ca");
		auto sorted  = results;
		std::sort(sorted.begin(), sorted.end());
		REQUIRE(results == sorted);
	}
}

// =============================================================================
//  TrieSearch vs HashSetSearch correctness
// =============================================================================
TEST_CASE("TrieSearch and HashSetSearch agree", "[trie][correctness]")
{
	DSALog::info("[trie] TrieSearch vs HashSetSearch");
	Trie::TrieSearch    trie_algo;
	Trie::HashSetSearch hash_algo;

	SECTION("all queries found")
	{
		Trie::TrieInput in{{"apple", "banana", "cherry"}, {"apple", "banana", "cherry"}};
		REQUIRE(trie_algo.run(in) == 3);
		REQUIRE(hash_algo.run(in) == 3);
	}

	SECTION("no queries found")
	{
		Trie::TrieInput in{{"apple", "banana"}, {"grape", "mango"}};
		REQUIRE(trie_algo.run(in) == 0);
		REQUIRE(hash_algo.run(in) == 0);
	}

	SECTION("partial hit")
	{
		Trie::TrieInput in{{"cat", "car", "card"}, {"cat", "care", "card"}};
		REQUIRE(trie_algo.run(in) == 2);
		REQUIRE(hash_algo.run(in) == 2);
	}

	SECTION("empty dictionary")
	{
		Trie::TrieInput in{{}, {"hello"}};
		REQUIRE(trie_algo.run(in) == 0);
		REQUIRE(hash_algo.run(in) == 0);
	}

	SECTION("empty queries")
	{
		Trie::TrieInput in{{"hello"}, {}};
		REQUIRE(trie_algo.run(in) == 0);
		REQUIRE(hash_algo.run(in) == 0);
	}

	SECTION("prefix is not a match without exact insertion")
	{
		Trie::TrieInput in{{"hello"}, {"hell", "hello", "helloo"}};
		REQUIRE(trie_algo.run(in) == 1);
		REQUIRE(hash_algo.run(in) == 1);
	}
}

// =============================================================================
//  TriePrefixCount vs LinearPrefixCount correctness
// =============================================================================
TEST_CASE("TriePrefixCount and LinearPrefixCount agree", "[trie][correctness]")
{
	DSALog::info("[trie] PrefixCount comparison");
	Trie::TriePrefixCount   trie_algo;
	Trie::LinearPrefixCount linear_algo;

	SECTION("prefix matches several words")
	{
		Trie::PrefixInput in{{"car", "card", "care", "cat", "dog"}, "car"};
		REQUIRE(trie_algo.run(in) == 3);
		REQUIRE(linear_algo.run(in) == 3);
	}

	SECTION("prefix matches all words")
	{
		Trie::PrefixInput in{{"abc", "abcd", "abcde"}, "abc"};
		REQUIRE(trie_algo.run(in) == 3);
		REQUIRE(linear_algo.run(in) == 3);
	}

	SECTION("prefix matches nothing")
	{
		Trie::PrefixInput in{{"apple", "banana"}, "xyz"};
		REQUIRE(trie_algo.run(in) == 0);
		REQUIRE(linear_algo.run(in) == 0);
	}

	SECTION("single-char prefix")
	{
		Trie::PrefixInput in{{"ant", "ape", "bear", "bee"}, "a"};
		REQUIRE(trie_algo.run(in) == 2);
		REQUIRE(linear_algo.run(in) == 2);
	}
}

// =============================================================================
//  Benchmarks
// =============================================================================
TEST_CASE("Trie benchmarks", "[trie][benchmark][!benchmark]")
{
	auto dict1k  = numbered_words(1000,  "word");
	auto dict10k = numbered_words(10000, "word");

	std::vector<std::string> queries1k;
	for (int i = 0; i < 500; ++i)  queries1k.push_back("word" + std::to_string(i));
	for (int i = 0; i < 500; ++i)  queries1k.push_back("miss" + std::to_string(i));

	std::vector<std::string> queries10k;
	for (int i = 0; i < 5000; ++i)  queries10k.push_back("word" + std::to_string(i));
	for (int i = 0; i < 5000; ++i)  queries10k.push_back("miss" + std::to_string(i));

	BENCHMARK("TrieSearch    dict=1k  queries=1k")
	{
		return Trie::TrieSearch{}.run({dict1k, queries1k});
	};
	BENCHMARK("HashSetSearch dict=1k  queries=1k")
	{
		return Trie::HashSetSearch{}.run({dict1k, queries1k});
	};
	BENCHMARK("TrieSearch    dict=10k queries=10k")
	{
		return Trie::TrieSearch{}.run({dict10k, queries10k});
	};
	BENCHMARK("HashSetSearch dict=10k queries=10k")
	{
		return Trie::HashSetSearch{}.run({dict10k, queries10k});
	};

	BENCHMARK("TriePrefixCount   dict=10k prefix=word1")
	{
		return Trie::TriePrefixCount{}.run({dict10k, "word1"});
	};
	BENCHMARK("LinearPrefixCount dict=10k prefix=word1")
	{
		return Trie::LinearPrefixCount{}.run({dict10k, "word1"});
	};
}
