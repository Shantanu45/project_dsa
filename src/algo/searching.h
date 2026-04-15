#pragma once
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