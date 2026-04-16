#pragma once
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
 * Quick Sort is widely considered one of the most efficient and popular sorting algorithms in computer science. 
 * While its worst-case scenario can be slow, its real-world performance usually beats out competitors like Merge Sort or Heap Sort.
 * 
 * Quick Sort is in-place
 * 
 * Execellent locality of reference. Since it accesses elements sequentially from the ends toward the middle (or linearly through the partition)
 * 
 * Parallelization Potential: because of its Divide and Conquer nature.
 * 
 * Skip when:
 * - Stability is required: If you need to keep elements with equal values in their original relative order, use Merge Sort.
 * - Worst-case protection
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
 * Merge sort is a popular sorting algorithm known for its efficiency and stability. It follows the Divide and Conquer approach. 
 * It works by recursively dividing the input array into two halves, recursively sorting the two halves and finally merging them back together to obtain the sorted array.
 * 
 * Divide and Conquer algorithm
 * 
 * Space complexity O(n): This is its main drawback
 * 
 * Benefits:
 * - Stability: Yes. Merge Sort preserves the relative order of equal elements, which is vital when sorting complex data (like sorting a list of people by "Last Name" then "First Name" or Sorting logs by "Severity Level," but keeping them in their original "Timestamp" order within those levels.).
 * - Predictable Performance: Merge Sort does not have a "worst-case" scenario based on data arrangement.
 * - External Sorting: Merge Sort is the king of data that is too big to fit in RAM.
 * - Perfect for Linked Lists (std::list::sort actually uses Merge Sort): Linked lists don't allow "random access" (you can't jump to index 500 instantly).
 * - Parallelization
 */
class MergeSort : public Algorithm<Vec, Vec>
{
  void ms(Vec &v, Vec &aux, int lo, int hi) const
  {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;

    ms(v, aux, lo, mid);
    ms(v, aux, mid + 1, hi);

    // MERGE STEP using the 'aux' scratchpad
    int i = lo, j = mid + 1, k = lo;

    while (i <= mid && j <= hi) aux[k++] = (v[i] <= v[j]) ? v[i++] : v[j++];
    while (i <= mid) aux[k++] = v[i++];
    while (j <= hi) aux[k++] = v[j++];

    // Copy back from aux to original
    for (int x = lo; x <= hi; ++x) v[x] = aux[x];
  }

public:
  Vec run(const Vec &input) override
  {
    Vec v = input;
    if (!v.empty()) {
      // Create the scratchpad once!
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
 * Heap Sort is a comparison-based sorting algorithm that uses a specific data structure called a Binary Heap (like Max heap or Min heap) to manage elements. 
 * If Merge Sort is "split and zip," and Quick Sort is "partitioning," then Heap Sort is "rank and extract."
 * 
 * It is essentially a more sophisticated version of Selection Sort
 * 
 * Benefit: 
 * - Guaranteed Performance: Unlike Quick Sort, which can fail to O(n2) if the pivot is bad, Heap Sort always finishes in O(nlogn).
 * - Memory Efficient: It doesn't use recursion (so no stack overflow risk) and requires zero extra memory (unlike Merge Sort).
 * - The "Top K" Advantage: If you only need the top 10 largest elements from a list of a billion, you don't need to sort the whole thing. You just "Heapify" and extract 10 times, making it incredibly efficient for priority-based tasks.
 * 
 * Drawback:
 * On average, it is usually slower than Quick Sort because the way it moves through memory (jumping from parent to child) isn't as "cache-friendly" for the CPU.
 * 
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

// -- InsertionSort -----------------------------------------------------------------
/**
 * Insertion sort is a simple sorting algorithm that works by iteratively inserting each element of an unsorted list into its correct position in a sorted portion of the list. 
 * It is like sorting playing cards in your hands.
 * 
 * Benefits:
 * - High Performance on "Nearly Sorted" Data : Insertion Sort is an adaptive algorithm. If the data is already sorted (or mostly sorted), it only does one comparison per element and moves on.
 * - Extremely Low Overhead: simple while loop with almost no setup.
 * - It is "Online": Insertion Sort can sort data as it receives it.
 * - Stability and In-Place Sorting
 * - Memory/Cache Efficiency
 */
class InsertionSort : public Algorithm<Vec, Vec>
{
  void is(Vec &v) {
    for (int i = 1; i < v.size(); i++) {
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
 * std::sort uses hybrid algorithm.
 * How it works: 
 * 1. It starts with Quick Sort.
 * 2. If the recursion depth gets too deep (risking O(n2)), it switches to Heap Sort.
 * 3. For very small arrays (e.g., < 16 elements), it switches to Insertion Sort.
 * 
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