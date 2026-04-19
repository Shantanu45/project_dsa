#pragma once
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  Trie namespace
// -----------------------------------------------------------------------------
namespace Trie
{

// =============================================================================
//  Trie  -  Prefix tree for lowercase-letter string keys
// =============================================================================
/**
 * A Trie stores strings character-by-character in a tree.  Each node has up
 * to 26 children (one per lowercase letter).  A boolean flag marks nodes that
 * complete a whole word.
 *
 * Complexities  (L = length of key,  n = number of words in trie)
 *   insert         O(L)
 *   search         O(L)
 *   starts_with    O(L)
 *   words_with_prefix  O(L + total chars in matching words)
 *   space          O(total characters across all inserted keys)
 *
 * Key advantages over std::unordered_set<string>:
 *   - Prefix queries are O(L) instead of O(n * L) linear scan
 *   - Natural lexicographic ordering (DFS = sorted output)
 *   - No hashing overhead; no collision handling
 *   - Shared prefixes (e.g. "car", "card", "care") use a single path
 *
 * Classic uses: autocomplete, spell-checker, IP routing, word games.
 */
class Trie
{
	struct Node
	{
		Node* children[26]{};
		bool  is_end = false;
		~Node() { for (auto* c : children) delete c; }
	};

	Node* root_;
	int   size_ = 0;

   public:
	Trie() : root_(new Node()) {}
	~Trie() { delete root_; }
	Trie(const Trie&)            = delete;
	Trie& operator=(const Trie&) = delete;

	void insert(const std::string& word)
	{
		Node* cur = root_;
		for (char c : word)
		{
			int idx = c - 'a';
			if (!cur->children[idx]) cur->children[idx] = new Node();
			cur = cur->children[idx];
		}
		if (!cur->is_end) { cur->is_end = true; ++size_; }
	}

	bool search(const std::string& word) const
	{
		const Node* cur = root_;
		for (char c : word)
		{
			int idx = c - 'a';
			if (!cur->children[idx]) return false;
			cur = cur->children[idx];
		}
		return cur->is_end;
	}

	// Returns true if any inserted word starts with 'prefix'
	bool starts_with(const std::string& prefix) const
	{
		const Node* cur = root_;
		for (char c : prefix)
		{
			int idx = c - 'a';
			if (!cur->children[idx]) return false;
			cur = cur->children[idx];
		}
		return true;
	}

	// Collect all inserted words that start with 'prefix' (DFS, lexicographic order)
	std::vector<std::string> words_with_prefix(const std::string& prefix) const
	{
		const Node* cur = root_;
		for (char c : prefix)
		{
			int idx = c - 'a';
			if (!cur->children[idx]) return {};
			cur = cur->children[idx];
		}
		std::vector<std::string> result;
		_collect(cur, prefix, result);
		return result;
	}

	int  size()  const { return size_; }
	bool empty() const { return size_ == 0; }

   private:
	void _collect(const Node* node, std::string current, std::vector<std::string>& out) const
	{
		if (node->is_end) out.push_back(current);
		for (int i = 0; i < 26; ++i)
			if (node->children[i])
				_collect(node->children[i], current + char('a' + i), out);
	}
};

// =============================================================================
//  Algorithm wrappers
// =============================================================================

struct TrieInput
{
	std::vector<std::string> words;    // dictionary to build
	std::vector<std::string> queries;  // exact-match queries
};

// -- TrieSearch: build trie then answer exact-match queries -------------------
/**
 * Builds a Trie from a dictionary, then answers a batch of exact-match
 * queries.  The result is the number of queries that hit.
 *
 * Compared against std::unordered_set as the hash-table baseline.
 * Both are O(L) per query after an O(n*L) build phase.
 * Trie wins when you also need prefix queries — the hash set can't do them.
 */
class TrieSearch : public Algorithm<TrieInput, int>
{
   public:
	int run(const TrieInput& in) override
	{
		Trie t;
		for (const auto& w : in.words) t.insert(w);
		int found = 0;
		for (const auto& q : in.queries)
			if (t.search(q)) ++found;
		return found;
	}
	std::string name()        const override { return "TrieSearch"; }
	std::string description() const override { return "O(L) per query after O(n*L) build"; }
	std::string complexity()  const override { return "Build O(n*L), Query O(L)"; }
};

class HashSetSearch : public Algorithm<TrieInput, int>
{
   public:
	int run(const TrieInput& in) override
	{
		std::unordered_set<std::string> s(in.words.begin(), in.words.end());
		int found = 0;
		for (const auto& q : in.queries)
			if (s.count(q)) ++found;
		return found;
	}
	std::string name()        const override { return "HashSetSearch"; }
	std::string description() const override { return "O(L) avg per query, hash-table baseline"; }
};

// -- TriePrefixCount: count words that match a given prefix -------------------
/**
 * The key benchmark for Trie: prefix queries.
 * Trie is O(L + k) where k is the number of matching words.
 * Linear scan is O(n * L) — it must check every word.
 *
 * This gap widens dramatically as the dictionary grows.
 */
struct PrefixInput
{
	std::vector<std::string> words;
	std::string              prefix;
};

class TriePrefixCount : public Algorithm<PrefixInput, int>
{
   public:
	int run(const PrefixInput& in) override
	{
		Trie t;
		for (const auto& w : in.words) t.insert(w);
		return (int)t.words_with_prefix(in.prefix).size();
	}
	std::string name()        const override { return "TriePrefixCount"; }
	std::string description() const override { return "O(L + matches) prefix enumeration"; }
	std::string complexity()  const override { return "O(L + k), k = matching words"; }
};

class LinearPrefixCount : public Algorithm<PrefixInput, int>
{
   public:
	int run(const PrefixInput& in) override
	{
		int count = 0;
		for (const auto& w : in.words)
			if (w.rfind(in.prefix, 0) == 0) ++count;
		return count;
	}
	std::string name()        const override { return "LinearPrefixCount"; }
	std::string description() const override { return "O(n*L) linear scan baseline"; }
};

}  // namespace Trie
