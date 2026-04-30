/*****************************************************************//**
 * \file   sorting.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <algorithm>
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  Sorting namespace
//  All algorithms here:  In  = std::vector<int>
//                        Out = std::vector<int>
// -----------------------------------------------------------------------------
namespace Sorting
{

using Vec = std::vector<int>;

// -- QuickSort ----------------------------------------------------------------
/**
 * @brief In-place divide-and-conquer sort using a partition pivot.
 *
 * @details
 * Lomuto partition scheme (pivot = last element):
 *   1. Rearrange the subarray so all elements ≤ pivot are on the left,
 *      all elements > pivot are on the right.  The pivot lands at its final
 *      sorted position after this step.
 *   2. Recursively apply to the two sub-partitions.
 *
 * The pivot choice (last element) is simple but produces O(n²) behaviour on
 * already-sorted input.  Production implementations use median-of-three or
 * random pivot selection to make this astronomically unlikely.
 *
 * @par Complexity
 *   Time  O(n log n) average — each element participates in O(log n) partitions.
 *   Time  O(n²) worst case  — occurs when pivot is always the min/max
 *                             (sorted or reverse-sorted input with this scheme).
 *   Space O(log n) average / O(n) worst — recursion stack depth.
 *   In-place: no auxiliary array needed.
 *
 * @par Advantages
 *   - Excellent cache locality: accesses elements sequentially within each
 *     partition, fitting naturally into CPU cache lines.
 *   - Low constant factor: minimal bookkeeping per comparison.
 *   - Natural parallelism: sub-partitions are independent.
 *
 * @par When to skip
 *   - Stability required -> use MergeSort.
 *   - Guaranteed worst-case needed -> use HeapSort or std::sort (Introsort).
 */
class QuickSort : public Algorithm<Vec, Vec>
{
	void qs(Vec& v, int lo, int hi) const
	{
		if (lo >= hi)
			return;
		int pivot = v[hi], i = lo - 1;
		for (int j = lo; j < hi; ++j)
			if (v[j] <= pivot)
				std::swap(v[++i], v[j]);
		std::swap(v[++i], v[hi]);
		qs(v, lo, i - 1);
		qs(v, i + 1, hi);
	}

   public:
	Vec run(const Vec& input) override
	{
		Vec v = input;
		if (!v.empty())
			qs(v, 0, (int)v.size() - 1);
		return v;
	}
	std::string name() const override { return "QuickSort"; }
	std::string description() const override { return "O(n log n) avg, O(n^2) worst"; }
};

// -- MergeSort ----------------------------------------------------------------
/**
 * @brief Stable divide-and-conquer sort with guaranteed O(n log n).
 *
 * @details
 * Recursive top-down variant with a shared auxiliary buffer allocated once:
 *   1. Divide: split [lo, hi] at mid = lo + (hi-lo)/2.
 *   2. Conquer: recursively sort [lo, mid] and [mid+1, hi].
 *   3. Merge: two-pointer merge of the two sorted halves into aux[lo..hi],
 *      then copy back.  The ≤ comparison in the merge preserves stability.
 *
 * The auxiliary array is allocated once at the top level and shared across
 * all recursive calls — avoiding O(n log n) total allocations.
 *
 * @par Complexity
 *   Time  O(n log n) — always, regardless of input order.
 *   Space O(n)       — auxiliary array for merging (main drawback).
 *
 * @par Advantages
 *   - Stable: equal elements preserve their original relative order.
 *   - Predictable: no worst-case degradation unlike QuickSort.
 *   - Optimal for linked lists: merging singly-linked lists needs no extra
 *     memory and no random access.
 *   - External sorting: can merge sorted chunks that don't fit in RAM by
 *     streaming from disk (e.g. sort 1 TB with 1 GB RAM).
 *   - Parallelisable: sub-problems are fully independent.
 *
 * @par When to skip
 *   - Memory is tight -> use HeapSort (O(1) extra space).
 *   - Raw speed on cache-friendly data -> QuickSort often faster in practice.
 */
class MergeSort : public Algorithm<Vec, Vec>
{
  void ms(Vec &v, Vec &aux, int lo, int hi) const
  {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;

    ms(v, aux, lo, mid);
    ms(v, aux, mid + 1, hi);

    // Merge the two sorted halves into aux, then copy back.
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) aux[k++] = (v[i] <= v[j]) ? v[i++] : v[j++];
    while (i <= mid) aux[k++] = v[i++];
    while (j <= hi) aux[k++] = v[j++];
    for (int x = lo; x <= hi; ++x) v[x] = aux[x];
  }

public:
  Vec run(const Vec &input) override
  {
    Vec v = input;
    if (!v.empty()) {
      Vec aux(v.size());
      ms(v, aux, 0, (int)v.size() - 1);
    }
    return v;
  }
  std::string name() const override { return "MergeSort"; }
  std::string description() const override { return "O(n log n) stable, O(n) space"; }
};

// -- HeapSort -----------------------------------------------------------------
/**
 * @brief In-place sort using a binary max-heap.
 *
 * @details
 * Two-phase algorithm, both phases use the sift-down (heapify) subroutine:
 *
 *   Phase 1 – Heapify (O(n)):
 *     Build a max-heap in-place using Floyd's bottom-up algorithm.
 *     Start from the last internal node (index n/2 - 1) and sift each node
 *     down.  This is O(n) — provably fewer comparisons than n individual
 *     pushes because most nodes are near the leaves and travel very little.
 *
 *   Phase 2 – Extract-sort (O(n log n)):
 *     Repeatedly swap the heap root (the current maximum) with the last
 *     element of the unsorted region, then sift-down the new root to restore
 *     the heap property over the remaining n-1 elements.
 *     Each extraction costs O(log n), so n extractions = O(n log n) total.
 *
 * The net effect: after Phase 2, the array is sorted ascending in-place.
 *
 * @par Complexity
 *   Time  O(n log n) — guaranteed, no bad pivot scenarios.
 *   Space O(1)       — purely in-place; no recursion stack (iterative sift-down).
 *   Not stable.
 *
 * @par Advantages
 *   - Guaranteed O(n log n) with O(1) extra space — best of both worlds vs
 *     QuickSort (bad worst case) and MergeSort (O(n) space).
 *   - No stack-overflow risk: iterative inner loop.
 *   - Useful for "top-k" extraction: heapify once, extract k times in O(n + k log n).
 *
 * @par Drawbacks
 *   - Poor cache locality: sift-down jumps between parent and child indices
 *     (non-sequential memory access), causing frequent cache misses.
 *   - In practice slower than QuickSort and often slower than MergeSort on
 *     modern hardware despite the same asymptotic bound.
 */
class HeapSort : public Algorithm<Vec, Vec>
{
	void heapify(Vec& v, int n, int i) const
	{
		int largest = i, l = 2 * i + 1, r = 2 * i + 2;
		if (l < n && v[l] > v[largest])
			largest = l;
		if (r < n && v[r] > v[largest])
			largest = r;
		if (largest != i)
		{
			std::swap(v[i], v[largest]);
			heapify(v, n, largest);
		}
	}

   public:
	Vec run(const Vec& input) override
	{
		Vec v = input;
		int n = (int)v.size();
		for (int i = n / 2 - 1; i >= 0; --i)
			heapify(v, n, i);
		for (int i = n - 1; i > 0; --i)
		{
			std::swap(v[0], v[i]);
			heapify(v, i, 0);
		}
		return v;
	}
	std::string name() const override { return "HeapSort"; }
	std::string description() const override { return "O(n log n) in-place, not stable"; }
};

// -- InsertionSort ------------------------------------------------------------
/**
 * @brief Stable sort that builds the sorted output one element at a time.
 *
 * @details
 * Analogous to sorting a hand of playing cards:
 *   For each element at index i (starting from i=1):
 *     1. Save the element as 'key'.
 *     2. Shift all elements to its left that are greater than key one position
 *        to the right, opening a gap.
 *     3. Insert key into the gap.
 *
 * After processing index i, arr[0..i] is sorted — the sorted prefix grows
 * by one element each iteration.
 *
 * @par Complexity
 *   Time  O(n²) worst/average — occurs on reverse-sorted input (every element
 *          shifts the entire prefix).
 *   Time  O(n) best case      — already-sorted input: no shifts needed (1
 *          comparison per element).  The algorithm is adaptive: proportional
 *          to the number of inversions, which is 0 for a sorted array.
 *   Space O(1) — in-place.
 *   Stable.
 *
 * @par Advantages
 *   - Fastest of all O(n²) sorts for nearly-sorted or small (n < ~20) input.
 *   - Online: can sort a stream one element at a time without seeing all data first.
 *   - Extremely low overhead: just comparisons and shifts, no division or allocation.
 *   - Used as the small-subarray routine inside Introsort (std::sort) and Timsort.
 *
 * @par When to skip
 *   Large arrays that are not already nearly sorted.
 */
class InsertionSort : public Algorithm<Vec, Vec>
{
  void is(Vec &v) {
    for (int i = 1; i < (int)v.size(); i++) {
      int key = v[i];
      int j = i - 1;
      while (j >= 0 && v[j] > key) {
        v[j + 1] = v[j];
        j = j - 1;
      }
      v[j + 1] = key;
    }
  }

public:
	Vec run(const Vec& input) override
	{
        Vec v = input;
        if (!v.empty()) is(v);
        return v;
	}

	std::string name() const override { return "InsertionSort"; }
	std::string description() const override { return "O(n^2) stable, O(1) space"; }
};

// -- std::sort wrapper (reference baseline) -----------------------------------
/**
 * @brief Introsort hybrid: QuickSort + HeapSort + InsertionSort.
 *
 * @details
 * std::sort in most standard library implementations uses Introsort
 * (introspective sort), a three-way hybrid that avoids each component's
 * weakness:
 *
 *   1. QuickSort by default — fast in practice, excellent cache behaviour.
 *   2. Switches to HeapSort if the recursion depth exceeds 2*log₂(n) —
 *      guarantees O(n log n) worst case even on adversarial pivot inputs.
 *   3. Falls back to InsertionSort for sub-arrays of size ≤ ~16 — minimises
 *      overhead for tiny ranges where recursive calls dominate.
 *
 * Not guaranteed to be stable (use std::stable_sort for that).
 *
 * @par Complexity
 *   Time  O(n log n) guaranteed.
 *   Space O(log n) — recursion stack.
 *   Not stable (implementation-defined).
 *
 * @par When to use
 *   Default choice for general-purpose sorting.  Use this as the performance
 *   baseline against which other sorting algorithms are compared.
 */
class StdSort : public Algorithm<Vec, Vec>
{
   public:
	Vec run(const Vec& input) override
	{
		Vec v = input;
		std::sort(v.begin(), v.end());
		return v;
	}
	std::string name() const override { return "std::sort"; }
};

}  // namespace Sorting
