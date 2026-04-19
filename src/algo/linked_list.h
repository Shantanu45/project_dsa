#pragma once
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  LinkedList namespace
// -----------------------------------------------------------------------------
namespace LinkedList
{

using Vec = std::vector<int>;

// =============================================================================
//  Node<T>
// =============================================================================
template <typename T>
struct Node
{
	T      val;
	Node*  next = nullptr;
	explicit Node(T v) : val(std::move(v)) {}
};

// =============================================================================
//  SinglyLinkedList<T>
// =============================================================================
/**
 * A singly linked list backed by heap-allocated nodes.
 *
 * Complexities
 *   push_front   O(1)
 *   push_back    O(1)   (tail pointer maintained)
 *   pop_front    O(1)
 *   search       O(n)
 *   reverse      O(n)   iterative, O(1) extra space — key advantage over array reverse
 *   find_middle  O(n)   via fast/slow (tortoise-and-hare) pointers
 *   has_cycle    O(n)   O(1) extra space — Floyd's algorithm
 *
 * Why linked lists?
 *   - O(1) insert/delete at a KNOWN pointer position (no shifting like arrays)
 *   - Natural for recursive structures, LRU caches, adjacency lists
 *   - Merge Sort is O(n log n) with O(1) extra space (merging needs no random access)
 *   - Trade-off: no random access, poor cache locality (pointer chasing)
 */
template <typename T>
class SinglyLinkedList
{
	Node<T>* head_ = nullptr;
	Node<T>* tail_ = nullptr;
	int      size_ = 0;

   public:
	SinglyLinkedList() = default;

	explicit SinglyLinkedList(const std::vector<T>& v) { for (const T& x : v) push_back(x); }

	~SinglyLinkedList() { clear(); }
	SinglyLinkedList(const SinglyLinkedList&)            = delete;
	SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

	void push_front(T val)
	{
		auto* n = new Node<T>(std::move(val));
		n->next = head_;
		head_   = n;
		if (!tail_) tail_ = head_;
		++size_;
	}

	void push_back(T val)
	{
		auto* n = new Node<T>(std::move(val));
		if (tail_) tail_->next = n;
		else       head_       = n;
		tail_ = n;
		++size_;
	}

	T pop_front()
	{
		if (!head_) throw std::out_of_range("List is empty");
		T        val = std::move(head_->val);
		Node<T>* old = head_;
		head_ = head_->next;
		if (!head_) tail_ = nullptr;
		delete old;
		--size_;
		return val;
	}

	// Iterative reversal — O(n), O(1) extra space.
	// Three-pointer technique: prev, cur, next.
	void reverse()
	{
		Node<T>* prev = nullptr;
		Node<T>* cur  = head_;
		tail_         = head_;
		while (cur)
		{
			Node<T>* nxt = cur->next;
			cur->next    = prev;
			prev         = cur;
			cur          = nxt;
		}
		head_ = prev;
	}

	// Floyd's tortoise-and-hare cycle detection — O(n), O(1) space.
	// If a cycle exists, fast pointer laps slow pointer; otherwise fast hits null.
	bool has_cycle() const
	{
		Node<T>* slow = head_;
		Node<T>* fast = head_;
		while (fast && fast->next)
		{
			slow = slow->next;
			fast = fast->next->next;
			if (slow == fast) return true;
		}
		return false;
	}

	// Find middle node via fast/slow pointers — O(n), O(1) space.
	// For even length, returns the lower-middle (e.g. index 1 for length 4).
	Node<T>* find_middle() const
	{
		if (!head_) return nullptr;
		Node<T>* slow = head_;
		Node<T>* fast = head_;
		while (fast->next && fast->next->next)
		{
			slow = slow->next;
			fast = fast->next->next;
		}
		return slow;
	}

	Node<T>* head()  const { return head_; }
	int      size()  const { return size_; }
	bool     empty() const { return size_ == 0; }

	std::vector<T> to_vector() const
	{
		std::vector<T> v;
		v.reserve(size_);
		for (Node<T>* cur = head_; cur; cur = cur->next)
			v.push_back(cur->val);
		return v;
	}

	void clear()
	{
		while (head_)
		{
			Node<T>* nxt = head_->next;
			delete head_;
			head_ = nxt;
		}
		tail_ = nullptr;
		size_ = 0;
	}
};

// =============================================================================
//  Algorithm wrappers
// =============================================================================

// -- ListReverse: linked list vs std::reverse ---------------------------------
/**
 * Both are O(n). The linked list reverse uses pointer manipulation with O(1)
 * extra space.  std::reverse on a vector swaps elements in-place — better
 * cache behaviour in practice due to contiguous memory.
 */
class ListReverse : public Algorithm<Vec, Vec>
{
   public:
	Vec run(const Vec& input) override
	{
		SinglyLinkedList<int> l(input);
		l.reverse();
		return l.to_vector();
	}
	std::string name()        const override { return "ListReverse"; }
	std::string description() const override { return "O(n) pointer reversal"; }
	std::string complexity()  const override { return "O(n) time, O(1) extra space"; }
};

class ArrayReverse : public Algorithm<Vec, Vec>
{
   public:
	Vec run(const Vec& input) override
	{
		Vec v = input;
		std::reverse(v.begin(), v.end());
		return v;
	}
	std::string name()        const override { return "ArrayReverse"; }
	std::string description() const override { return "O(n) std::reverse baseline"; }
};

// -- MergeSortedLists: merge two sorted sequences -----------------------------
/**
 * Classic O(m+n) two-pointer merge, performed by advancing through two
 * linked lists and appending the smaller head each time.
 *
 * The pointer-chasing version has worse cache behaviour than std::merge on
 * contiguous vectors, so std::merge wins on benchmarks — but the linked list
 * version needs zero extra allocation (it relinks existing nodes).
 * Here we collect into a vector for a fair output comparison.
 */
struct MergeInput { Vec a, b; };

class ListMergeSorted : public Algorithm<MergeInput, Vec>
{
   public:
	Vec run(const MergeInput& in) override
	{
		SinglyLinkedList<int> la(in.a), lb(in.b);
		Vec result;
		result.reserve(in.a.size() + in.b.size());
		Node<int>* a = la.head();
		Node<int>* b = lb.head();
		while (a && b)
		{
			if (a->val <= b->val) { result.push_back(a->val); a = a->next; }
			else                  { result.push_back(b->val); b = b->next; }
		}
		while (a) { result.push_back(a->val); a = a->next; }
		while (b) { result.push_back(b->val); b = b->next; }
		return result;
	}
	std::string name()        const override { return "ListMerge"; }
	std::string description() const override { return "O(m+n) two-pointer merge via list"; }
	std::string complexity()  const override { return "O(m+n) time, O(1) extra space"; }
};

class ArrayMergeSorted : public Algorithm<MergeInput, Vec>
{
   public:
	Vec run(const MergeInput& in) override
	{
		Vec result;
		result.reserve(in.a.size() + in.b.size());
		std::merge(in.a.begin(), in.a.end(), in.b.begin(), in.b.end(), std::back_inserter(result));
		return result;
	}
	std::string name()        const override { return "ArrayMerge"; }
	std::string description() const override { return "O(m+n) std::merge baseline"; }
};

}  // namespace LinkedList
