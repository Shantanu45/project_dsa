#pragma once
#include <algorithm>
#include <deque>
#include <vector>
#include "dsa_framework/framework.h"

namespace MonotonicQueue {

// =============================================================================
//  MonotonicDeque  –  sliding-window max/min in O(1) amortised per element
// =============================================================================
/**
 * Classic trick: maintain a deque of *indices* whose values are monotonically
 * decreasing (for max) or increasing (for min).
 *
 * Invariant (max-mode):
 *   arr[dq.front()] >= arr[dq[1]] >= ... >= arr[dq.back()]
 *   The front is always the index of the current window maximum.
 *
 * On push(i):
 *   Pop indices from the back while their value <= arr[i]  (they can never
 *   be the future maximum while i is still in the window).
 *   Then push_back(i).
 *
 * On evict(i):  (called when index i leaves the window)
 *   If i == front, pop_front.  Otherwise i is already gone (it was popped
 *   during a previous push because a larger element arrived).
 *
 * Amortised O(n) total: each index is pushed and popped at most once.
 */
class MonotonicDeque
{
    std::deque<int>         dq_;
    const std::vector<int>& arr_;
    bool                    max_mode_;

   public:
    MonotonicDeque(const std::vector<int>& arr, bool max_mode = true)
        : arr_(arr), max_mode_(max_mode)
    {}

    void push(int idx)
    {
        if (max_mode_)
            while (!dq_.empty() && arr_[dq_.back()] <= arr_[idx]) dq_.pop_back();
        else
            while (!dq_.empty() && arr_[dq_.back()] >= arr_[idx]) dq_.pop_back();
        dq_.push_back(idx);
    }

    void evict(int idx)
    {
        if (!dq_.empty() && dq_.front() == idx) dq_.pop_front();
    }

    int  front_val() const { return arr_[dq_.front()]; }
    bool empty()     const { return dq_.empty(); }
};

// =============================================================================
//  Input / Output types
// =============================================================================
struct SlidingInput
{
    std::vector<int> arr;
    int              k;    // window size
};
using SlidingOutput = std::vector<int>;

// =============================================================================
//  SlidingWindowMaxDeque  –  O(n) using MonotonicDeque
// =============================================================================
struct SlidingWindowMaxDeque : Algorithm<SlidingInput, SlidingOutput>
{
    SlidingOutput run(const SlidingInput& in) override
    {
        const auto& arr = in.arr;
        int n = (int)arr.size(), k = in.k;
        if (n == 0 || k <= 0) return {};
        SlidingOutput result;
        result.reserve(n - k + 1);
        MonotonicDeque dq(arr, true);
        for (int i = 0; i < n; ++i)
        {
            dq.push(i);
            if (i >= k) dq.evict(i - k);
            if (i >= k - 1) result.push_back(dq.front_val());
        }
        return result;
    }
    std::string name()       const override { return "MonotonicDeque"; }
    std::string complexity() const override { return "O(n) time, O(k) space"; }
};

// =============================================================================
//  SlidingWindowMaxNaive  –  O(n*k) brute force
// =============================================================================
struct SlidingWindowMaxNaive : Algorithm<SlidingInput, SlidingOutput>
{
    SlidingOutput run(const SlidingInput& in) override
    {
        const auto& arr = in.arr;
        int n = (int)arr.size(), k = in.k;
        if (n == 0 || k <= 0) return {};
        SlidingOutput result;
        result.reserve(n - k + 1);
        for (int i = 0; i <= n - k; ++i)
            result.push_back(*std::max_element(arr.begin() + i, arr.begin() + i + k));
        return result;
    }
    std::string name()       const override { return "NaiveSliding"; }
    std::string complexity() const override { return "O(n*k) time"; }
};

// =============================================================================
//  SlidingWindowMinDeque  –  O(n) min variant
// =============================================================================
struct SlidingWindowMinDeque : Algorithm<SlidingInput, SlidingOutput>
{
    SlidingOutput run(const SlidingInput& in) override
    {
        const auto& arr = in.arr;
        int n = (int)arr.size(), k = in.k;
        if (n == 0 || k <= 0) return {};
        SlidingOutput result;
        result.reserve(n - k + 1);
        MonotonicDeque dq(arr, false);
        for (int i = 0; i < n; ++i)
        {
            dq.push(i);
            if (i >= k) dq.evict(i - k);
            if (i >= k - 1) result.push_back(dq.front_val());
        }
        return result;
    }
    std::string name()       const override { return "MonotonicDequeMin"; }
    std::string complexity() const override { return "O(n) time, O(k) space"; }
};

}  // namespace MonotonicQueue
