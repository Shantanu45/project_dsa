/*****************************************************************//**
 * \file   monotonic_queue.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <algorithm>
#include <deque>
#include <vector>
#include "dsa_framework/framework.h"

namespace MonotonicQueue {

// =============================================================================
//  MonotonicDeque  –  sliding-window max/min in O(1) amortised per element
// =============================================================================
/**
 * @brief Deque-based structure for O(1) amortised sliding-window max or min.
 *
 * @details
 * Classic deque trick: maintain a deque of *indices* whose corresponding
 * values form a monotonically decreasing sequence (max-mode) or increasing
 * sequence (min-mode).
 *
 * @par Invariant (max-mode)
 *   arr[dq.front()] >= arr[dq[1]] >= … >= arr[dq.back()]
 *   The front is always the index of the current window maximum.
 *
 * @par push(i) — O(1) amortised
 *   Pop indices from the back while their value ≤ arr[i] (they can never
 *   be the future maximum while i is still in the window — i dominates them).
 *   Then push_back(i).
 *
 * @par evict(i) — O(1)
 *   Called when index i leaves the sliding window.  If i == front (i.e. it
 *   was the current maximum), pop_front.  If i was already popped during a
 *   previous push (because a larger element arrived), it's no longer in the
 *   deque — nothing to do.
 *
 * @par Why O(n) total?
 *   Each index is pushed exactly once and popped at most once (either by
 *   evict or by a later push), so the total work across n elements is O(n).
 *
 * @par Space
 *   O(k) — the deque holds at most k indices (the window size).
 *
 * @note The deque stores indices, not values, so that evict() can compare
 *   the front against the outgoing index efficiently.
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
/**
 * @brief Sliding window maximum in O(n) using a monotonic deque.
 *
 * @details
 * For each window position i (window = arr[i-k+1 .. i]):
 *   1. push(i)      — add the new element, maintaining the deque invariant.
 *   2. evict(i-k)   — remove the element that just fell out of the window
 *                     (only relevant once i >= k).
 *   3. front_val()  — the deque front is the current window maximum.
 *
 * The output vector has n-k+1 elements (one per valid window position).
 *
 * @par Complexity
 *   Time  O(n) — each index is pushed and popped at most once.
 *   Space O(k) — the deque holds at most k indices.
 *
 * @par Classic uses
 *   LeetCode 239 (Sliding Window Maximum), temperature forecasting, stock
 *   price analysis, signal processing peak detection.
 */
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
/**
 * @brief Sliding window maximum — O(n×k) brute-force baseline.
 *
 * @details
 * For each window [i, i+k-1], scan all k elements and take the maximum.
 * Simple and obviously correct; used to validate the O(n) deque approach.
 *
 * @par Complexity
 *   Time  O(n × k).   Space O(1) extra.
 */
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
/**
 * @brief Sliding window minimum in O(n) using a monotonic deque (min-mode).
 *
 * @details
 * Identical algorithm to SlidingWindowMaxDeque except the deque is
 * initialised in min-mode (increasing order), so front_val() returns the
 * current window minimum instead of the maximum.
 *
 * @par Complexity
 *   Time  O(n).   Space O(k).
 *
 * @par Classic uses
 *   Shortest subarray with property, constraint propagation in scheduling,
 *   minimum cost sliding window in dynamic programming optimisations.
 */
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
