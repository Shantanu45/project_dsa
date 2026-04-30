/*****************************************************************//**
 * \file   searching.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  Searching namespace
//  In  = SearchInput { sorted array + target }
//  Out = int  (index, or -1 if not found)
// -----------------------------------------------------------------------------
namespace Searching
{

struct SearchInput
{
	std::vector<int> arr;
	int target;
};

using SAlgo = Algorithm<SearchInput, int>;

// -- Binary Search -------------------------------------------------------------
/**
 * @brief Iterative binary search on a sorted array.
 *
 * @details
 * Repeatedly halves the search interval by comparing the target against the
 * midpoint element:
 *   - If arr[mid] == target  -> found, return mid.
 *   - If arr[mid] <  target  -> target must be in the right half (lo = mid+1).
 *   - If arr[mid] >  target  -> target must be in the left half  (hi = mid-1).
 * The interval collapses to empty (lo > hi) when the target is absent.
 *
 * Mid is computed as lo + (hi-lo)/2 rather than (lo+hi)/2 to avoid signed
 * integer overflow when lo and hi are both large.
 *
 * @par Complexity
 *   Time  O(log n) — interval halves each iteration.
 *   Space O(1)     — iterative; no recursion stack.
 *
 * @par Precondition
 *   arr must be sorted in non-decreasing order.
 *
 * @par When to use
 *   Default first choice for searching in any sorted random-access container.
 *   Prefer over linear scan for n > ~20.
 *
 * @returns Index of target in arr, or -1 if not present.
 */
class BinarySearch : public SAlgo
{
   public:
	int run(const SearchInput& in) override
	{
		int lo = 0, hi = (int)in.arr.size() - 1;
		while (lo <= hi)
		{
			int mid = lo + (hi - lo) / 2;
			if (in.arr[mid] == in.target)
				return mid;
			else if (in.arr[mid] < in.target)
				lo = mid + 1;
			else
				hi = mid - 1;
		}
		return -1;
	}
	std::string name() const override { return "BinarySearch"; }
	std::string description() const override { return "O(log n), requires sorted input"; }
};

// -- Linear Search -------------------------------------------------------------
/**
 * @brief Sequential scan from the first element to the last.
 *
 * @details
 * Examines every element in order and returns the index of the first match.
 * No preconditions on input ordering.
 *
 * Despite the poor worst-case complexity, linear search wins in practice for:
 *   - Very small arrays (n < ~10): no branch mispredictions, no division.
 *   - Unsorted data where sorting would cost more than a single scan.
 *   - Linked lists and other non-random-access structures.
 *
 * @par Complexity
 *   Time  O(n) worst case — scans the whole array if target is absent or last.
 *   Space O(1).
 *
 * @par When to use
 *   Small or unsorted collections, or when a single search is cheaper than
 *   the O(n log n) cost of sorting first.
 *
 * @returns Index of first occurrence of target, or -1 if not present.
 */
class LinearSearch : public SAlgo
{
   public:
	int run(const SearchInput& in) override
	{
		for (int i = 0; i < (int)in.arr.size(); ++i)
			if (in.arr[i] == in.target)
				return i;
		return -1;
	}
	std::string name() const override { return "LinearSearch"; }
	std::string description() const override { return "O(n), works on unsorted input"; }
};

// -- Interpolation Search ------------------------------------------------------
/**
 * @brief Improved binary search that guesses the probe position from value distribution.
 *
 * @details
 * Instead of always probing the midpoint, interpolation search estimates the
 * likely position of the target based on how it compares to the endpoints:
 *
 *   pos = lo + (hi - lo) * (target - arr[lo]) / (arr[hi] - arr[lo])
 *
 * This is equivalent to linear interpolation: if the array were perfectly
 * uniformly distributed, this formula would land exactly on the target on the
 * first probe.
 *
 * On each iteration the interval shrinks based on the computed probe position
 * rather than always halving — this makes it faster than binary search for
 * uniform data but can degrade to O(n) for skewed distributions (e.g. an
 * exponentially growing sequence where one half is always tiny).
 *
 * @par Complexity
 *   Time  O(log log n) average on uniformly distributed data.
 *   Time  O(n) worst case (non-uniform distribution).
 *   Space O(1).
 *
 * @par Precondition
 *   arr must be sorted in non-decreasing order.
 *   Works best when elements are uniformly distributed (e.g. IDs, timestamps).
 *
 * @par When to use
 *   Large, uniformly distributed, sorted arrays where the O(log log n) constant
 *   factor matters.  Avoid for integer sequences with clustering or large gaps.
 *
 * @returns Index of target in arr, or -1 if not present.
 */
class InterpolationSearch : public SAlgo
{
   public:
	int run(const SearchInput& in) override
	{
		const auto& a = in.arr;
		int lo = 0, hi = (int)a.size() - 1;
		while (lo <= hi && in.target >= a[lo] && in.target <= a[hi])
		{
			if (lo == hi)
				return (a[lo] == in.target) ? lo : -1;
			int pos = lo + (int)(((double)(hi - lo) / (a[hi] - a[lo])) * (in.target - a[lo]));
			if (a[pos] == in.target)
				return pos;
			else if (a[pos] < in.target)
				lo = pos + 1;
			else
				hi = pos - 1;
		}
		return -1;
	}
	std::string name() const override { return "InterpolationSearch"; }
	std::string description() const override { return "O(log log n) avg on uniform data"; }
};

}  // namespace Searching
