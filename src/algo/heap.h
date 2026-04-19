#pragma once
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "dsa_framework/framework.h"

namespace Heap
{

using Vec = std::vector<int>;

// =============================================================================
//  MaxHeap<T>  -  Binary max-heap backed by a std::vector
// =============================================================================
/**
 * A Max-Heap is a complete binary tree stored in an array where every parent
 * node is >= its children.  The root always holds the largest element.
 *
 * Array layout:  parent(i) = (i-1)/2,  left(i) = 2i+1,  right(i) = 2i+2
 *
 * Complexities
 *   push      O(log n)  – sift-up from leaf to root
 *   pop       O(log n)  – swap root with last, then sift-down
 *   top/peek  O(1)
 *   heapify   O(n)      – Floyd's bottom-up algorithm (not O(n log n)!)
 *
 * Use when: you repeatedly need the maximum; implementing HeapSort;
 *           the "lower half" side of a MedianHeap.
 */
template <typename T>
class MaxHeap
{
	std::vector<T> data_;

	void sift_up(int i)
	{
		while (i > 0)
		{
			int p = (i - 1) / 2;		// Parent
			if (data_[p] >= data_[i]) break;		// If parent is already bigger, we are done!
			std::swap(data_[p], data_[i]);			// Otherwise, swap child and parent
			i = p;									// Move our focus up to the parent's old position
		}
	}

	void sift_down(int i)
	{
		int n = (int)data_.size();
		while (true)
		{
			int largest = i;
            int l = 2 * i + 1;			// left
            int r = 2 * i + 2;			// right
			if (l < n && data_[l] > data_[largest]) largest = l;
			if (r < n && data_[r] > data_[largest]) largest = r;
			if (largest == i) break;
			std::swap(data_[i], data_[largest]);
			i = largest;
		}
	}

   public:
	MaxHeap() = default;

	// Build heap from an existing array in O(n) via Floyd's algorithm.
	// Starts from the last internal node and sifts each one down — this is
	// provably O(n) because most nodes are near the leaves and travel little.
	explicit MaxHeap(std::vector<T> v) : data_(std::move(v))
	{
        // (size / 2 - 1) is the index of the last parent node
		for (int i = (int)data_.size() / 2 - 1; i >= 0; --i)
			sift_down(i);
	}

	void push(T val)
	{
		data_.push_back(std::move(val));
		sift_up((int)data_.size() - 1);
	}

	// Remove and return the maximum element.
	T pop()
	{
		if (data_.empty()) throw std::out_of_range("MaxHeap is empty");
		T top = std::move(data_[0]);
		data_[0] = std::move(data_.back());
		data_.pop_back();
		if (!data_.empty()) sift_down(0);
		return top;
	}

	const T& top() const
	{
		if (data_.empty()) throw std::out_of_range("MaxHeap is empty");
		return data_[0];
	}

	bool   empty() const { return data_.empty(); }
	size_t size()  const { return data_.size(); }

	// Repeatedly pop to get elements in ascending order.
	std::vector<T> drain_sorted()
	{
		std::vector<T> out;
		out.reserve(data_.size());
		while (!empty()) out.push_back(pop());
		std::reverse(out.begin(), out.end());
		return out;
	}
};

// =============================================================================
//  MinHeap<T>  -  Binary min-heap backed by a std::vector
// =============================================================================
/**
 * A Min-Heap is a complete binary tree stored in an array where every parent
 * node is <= its children.  The root always holds the smallest element.
 *
 * Complexities  (same as MaxHeap, different comparison direction)
 *   push      O(log n)
 *   pop       O(log n)
 *   top/peek  O(1)
 *   heapify   O(n)
 *
 * Use when: implementing a priority queue where lowest value = highest priority;
 *           the "upper half" side of a MedianHeap;
 *           TopK / KthLargest problems (maintain a size-k window of the largest seen).
 */
template <typename T>
class MinHeap
{
	std::vector<T> data_;

	void sift_up(int i)
	{
		while (i > 0)
		{
			int p = (i - 1) / 2;
			if (data_[p] <= data_[i]) break;
			std::swap(data_[p], data_[i]);
			i = p;
		}
	}

	void sift_down(int i)
	{
		int n = (int)data_.size();
		while (true)
		{
			int smallest = i;
			int l = 2 * i + 1, r = 2 * i + 2;
			if (l < n && data_[l] < data_[smallest]) smallest = l;
			if (r < n && data_[r] < data_[smallest]) smallest = r;
			if (smallest == i) break;
			std::swap(data_[i], data_[smallest]);
			i = smallest;
		}
	}

   public:
	MinHeap() = default;

	explicit MinHeap(std::vector<T> v) : data_(std::move(v))
	{
		for (int i = (int)data_.size() / 2 - 1; i >= 0; --i)
			sift_down(i);
	}

	void push(T val)
	{
		data_.push_back(std::move(val));
		sift_up((int)data_.size() - 1);
	}

	T pop()
	{
		if (data_.empty()) throw std::out_of_range("MinHeap is empty");
		T top = std::move(data_[0]);
		data_[0] = std::move(data_.back());
		data_.pop_back();
		if (!data_.empty()) sift_down(0);
		return top;
	}

	const T& top() const
	{
		if (data_.empty()) throw std::out_of_range("MinHeap is empty");
		return data_[0];
	}

	bool   empty() const { return data_.empty(); }
	size_t size()  const { return data_.size(); }

	std::vector<T> drain_sorted()
	{
		std::vector<T> out;
		out.reserve(data_.size());
		while (!empty()) out.push_back(pop());
		return out;
	}
};

// =============================================================================
//  MedianHeap  -  Two-heap trick for O(1) median queries
// =============================================================================
/**
 * Maintains two heaps that together partition all inserted elements:
 *   lo_  (MaxHeap) – the lower half;  lo_.top() is the lower median
 *   hi_  (MinHeap) – the upper half;  hi_.top() is the upper median
 *
 * Invariants after every push:
 *   1. lo_.top() <= hi_.top()          (partition is correct)
 *   2. |lo_.size() - hi_.size()| <= 1  (sizes stay balanced)
 *
 * Median:
 *   even count  → average of lo_.top() and hi_.top()
 *   odd count   → lo_.top()  (lo always holds the extra element)
 *
 * Complexities
 *   push   O(log n)  – at most two heap operations
 *   median O(1)
 *
 * Classic use-case: "Find median from a data stream" (LeetCode 295).
 */
class MedianHeap
{
	MaxHeap<int> lo_;  // lower half
	MinHeap<int> hi_;  // upper half

   public:
	void push(int val)
	{
		if (lo_.empty() || val <= lo_.top())
			lo_.push(val);
		else
			hi_.push(val);

		// Rebalance: lo_ may hold at most one more element than hi_
		if (lo_.size() > hi_.size() + 1)
			hi_.push(lo_.pop());
		else if (hi_.size() > lo_.size())
			lo_.push(hi_.pop());
	}

	double median() const
	{
		if (lo_.empty()) throw std::out_of_range("MedianHeap is empty");
		if (lo_.size() == hi_.size())
			return (lo_.top() + (double)hi_.top()) / 2.0;
		return lo_.top();
	}

	size_t size()  const { return lo_.size() + hi_.size(); }
	bool   empty() const { return lo_.empty() && hi_.empty(); }
};

// =============================================================================
//  Algorithm wrappers (plug into TestSuite / Benchmarker)
// =============================================================================

// -- Input types --------------------------------------------------------------
struct TopKInput { Vec arr; int k; };
struct KthInput  { Vec arr; int k; };

// -- TopK: return the K largest elements (sorted descending) ------------------
/**
 * Maintain a MinHeap of size k.  For each element:
 *   - push it onto the heap
 *   - if the heap exceeds k, evict the current minimum
 *
 * At the end, the heap contains exactly the k largest elements seen.
 * The minimum of the heap is the Kth largest overall.
 *
 * Why MinHeap for "largest"?  We want to quickly evict the smallest of our
 * current top-k candidates — that's the heap's minimum, so MinHeap is right.
 *
 * Time  O(n log k)   Space O(k)
 * Much faster than full sort when k << n.
 */
class TopKElements : public Algorithm<TopKInput, Vec>
{
   public:
	Vec run(const TopKInput& in) override
	{
		MinHeap<int> h;
		for (int x : in.arr)
		{
			h.push(x);
			if ((int)h.size() > in.k) h.pop();
		}
		Vec result;
		result.reserve(in.k);
		while (!h.empty()) result.push_back(h.pop());
		std::sort(result.rbegin(), result.rend());
		return result;
	}
	std::string name()        const override { return "TopKHeap"; }
	std::string description() const override { return "O(n log k) MinHeap of size k"; }
	std::string complexity()  const override { return "O(n log k), Space O(k)"; }

	bool validate(const Vec& got, const Vec& expected) const override
	{
		Vec g = got, e = expected;
		std::sort(g.rbegin(), g.rend());
		std::sort(e.rbegin(), e.rend());
		return g == e;
	}
};

// -- TopKSort: full-sort baseline ---------------------------------------------
class TopKSort : public Algorithm<TopKInput, Vec>
{
   public:
	Vec run(const TopKInput& in) override
	{
		Vec v = in.arr;
		std::sort(v.rbegin(), v.rend());
		v.resize(in.k);
		return v;
	}
	std::string name()        const override { return "TopKSort"; }
	std::string description() const override { return "O(n log n) full-sort baseline"; }

	bool validate(const Vec& got, const Vec& expected) const override
	{
		Vec g = got, e = expected;
		std::sort(g.rbegin(), g.rend());
		std::sort(e.rbegin(), e.rend());
		return g == e;
	}
};

// -- KthLargest: find the Kth largest element ---------------------------------
/**
 * Same MinHeap-of-size-k trick as TopK.  After scanning the full array,
 * the heap's minimum is by definition the Kth largest element.
 *
 * Time  O(n log k)   Space O(k)
 *
 * Compared against std::nth_element which is O(n) average via introselect —
 * the heap approach wins when k is small and n is huge (streaming context),
 * while nth_element wins in bulk/offline scenarios.
 */
class KthLargestHeap : public Algorithm<KthInput, int>
{
   public:
	int run(const KthInput& in) override
	{
		MinHeap<int> h;
		for (int x : in.arr)
		{
			h.push(x);
			if ((int)h.size() > in.k) h.pop();
		}
		return h.top();
	}
	std::string name()        const override { return "KthLargestHeap"; }
	std::string description() const override { return "O(n log k) MinHeap of size k"; }
	std::string complexity()  const override { return "O(n log k), Space O(k)"; }
};

// -- KthLargestNth: std::nth_element baseline ---------------------------------
class KthLargestNth : public Algorithm<KthInput, int>
{
   public:
	int run(const KthInput& in) override
	{
		Vec v = in.arr;
		int pos = (int)v.size() - in.k;
		std::nth_element(v.begin(), v.begin() + pos, v.end());
		return v[pos];
	}
	std::string name()        const override { return "KthLargestNth"; }
	std::string description() const override { return "O(n) avg via std::nth_element"; }
	std::string complexity()  const override { return "O(n) avg, O(n^2) worst"; }
};

// -- MedianFinder: static array median via MedianHeap -------------------------
/**
 * Feeds all elements one by one into a MedianHeap, then reads the O(1) median.
 * Total cost O(n log n) — same asymptotic as sort — but the two-heap approach
 * shines in streaming scenarios where the median must be available after every
 * insertion, not just at the end.
 *
 * Baseline: sort the array and index the middle — also O(n log n) but with
 * better constants due to cache-friendly access patterns.
 */
class MedianFinderHeap : public Algorithm<Vec, double>
{
   public:
	double run(const Vec& input) override
	{
		MedianHeap mh;
		for (int x : input) mh.push(x);
		return mh.median();
	}
	std::string name()        const override { return "MedianHeap"; }
	std::string description() const override { return "O(n log n) two-heap running median"; }
	std::string complexity()  const override { return "O(n log n), Space O(n)"; }
};

class MedianFinderSort : public Algorithm<Vec, double>
{
   public:
	double run(const Vec& input) override
	{
		Vec v = input;
		std::sort(v.begin(), v.end());
		int n = (int)v.size();
		return (n % 2 == 0) ? (v[n / 2 - 1] + (double)v[n / 2]) / 2.0 : v[n / 2];
	}
	std::string name()        const override { return "MedianSort"; }
	std::string description() const override { return "O(n log n) sort-then-index baseline"; }
};

// =============================================================================
//  IndexedMinHeap  -  Priority queue with O(log n) decrease-key
// =============================================================================
/**
 * A plain MinHeap can't efficiently update an arbitrary element's priority
 * because it doesn't know where that element sits in the heap array.
 *
 * IndexedMinHeap maintains a position map  pos_[id] → heap index, so
 * decrease_key can locate the entry in O(1) and sift it up in O(log n).
 *
 * This is the data structure needed for optimal Dijkstra and Prim's MST:
 *   - Without it: push duplicates, O((V + E) log E) total
 *   - With it:    decrease_key in place, O((V + E) log V) total
 *
 * API
 *   push(id, priority)          O(log n)  — insert or decrease if already present
 *   pop_min()                   O(log n)  — remove and return {id, priority} of min
 *   decrease_key(id, priority)  O(log n)  — update priority downward (no-op if higher)
 *   peek_min()                  O(1)
 *   contains(id)                O(1)
 *
 * Valid IDs: integers in [0, max_id).
 */
class IndexedMinHeap
{
	struct Entry { int id, priority; };
	std::vector<Entry> heap_;
	std::vector<int>   pos_;  // pos_[id] = index in heap_, or -1 if absent

	void _swap(int i, int j)
	{
		pos_[heap_[i].id] = j;
		pos_[heap_[j].id] = i;
		std::swap(heap_[i], heap_[j]);
	}

	void _sift_up(int i)
	{
		while (i > 0)
		{
			int p = (i - 1) / 2;
			if (heap_[p].priority <= heap_[i].priority) break;
			_swap(p, i);
			i = p;
		}
	}

	void _sift_down(int i)
	{
		int n = (int)heap_.size();
		while (true)
		{
			int s = i;
			int l = 2 * i + 1, r = 2 * i + 2;
			if (l < n && heap_[l].priority < heap_[s].priority) s = l;
			if (r < n && heap_[r].priority < heap_[s].priority) s = r;
			if (s == i) break;
			_swap(i, s);
			i = s;
		}
	}

   public:
	explicit IndexedMinHeap(int max_id) : pos_(max_id, -1) {}

	// Insert with priority, or decrease_key if id already present.
	void push(int id, int priority)
	{
		if (contains(id)) { decrease_key(id, priority); return; }
		pos_[id] = (int)heap_.size();
		heap_.push_back({id, priority});
		_sift_up(pos_[id]);
	}

	// Remove and return {id, priority} of the minimum-priority element.
	std::pair<int, int> pop_min()
	{
		if (heap_.empty()) throw std::out_of_range("IndexedMinHeap is empty");
		Entry top = heap_[0];
		_swap(0, (int)heap_.size() - 1);
		pos_[top.id] = -1;
		heap_.pop_back();
		if (!heap_.empty()) _sift_down(0);
		return {top.id, top.priority};
	}

	// Lower an existing element's priority.  No-op if new_priority >= current.
	void decrease_key(int id, int new_priority)
	{
		if (!contains(id)) throw std::invalid_argument("id not in heap");
		if (new_priority >= heap_[pos_[id]].priority) return;
		heap_[pos_[id]].priority = new_priority;
		_sift_up(pos_[id]);
	}

	std::pair<int, int> peek_min() const
	{
		if (heap_.empty()) throw std::out_of_range("IndexedMinHeap is empty");
		return {heap_[0].id, heap_[0].priority};
	}

	bool contains(int id) const { return id >= 0 && id < (int)pos_.size() && pos_[id] != -1; }
	bool empty()          const { return heap_.empty(); }
	int  size()           const { return (int)heap_.size(); }
};

}  // namespace Heap
