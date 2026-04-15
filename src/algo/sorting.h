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
	std::string description() const override { return "O(n log n) avg, O(n²) worst"; }
};

// -- MergeSort ----------------------------------------------------------------
class MergeSort : public Algorithm<Vec, Vec>
{
	void ms(Vec& v, int lo, int hi) const
	{
		if (lo >= hi)
			return;
		int mid = (lo + hi) / 2;
		ms(v, lo, mid);
		ms(v, mid + 1, hi);
		Vec tmp;
		int i = lo, j = mid + 1;
		while (i <= mid && j <= hi)
			tmp.push_back(v[i] <= v[j] ? v[i++] : v[j++]);
		while (i <= mid)
			tmp.push_back(v[i++]);
		while (j <= hi)
			tmp.push_back(v[j++]);
		for (int k = lo; k <= hi; ++k)
			v[k] = tmp[k - lo];
	}

   public:
	Vec run(const Vec& input) override
	{
		Vec v = input;
		if (!v.empty())
			ms(v, 0, (int)v.size() - 1);
		return v;
	}
	std::string name() const override { return "MergeSort"; }
	std::string description() const override { return "O(n log n) stable, O(n) space"; }
};

// -- HeapSort -----------------------------------------------------------------
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

// -- std::sort wrapper (reference baseline) -----------------------------------
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