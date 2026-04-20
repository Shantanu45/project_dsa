/*****************************************************************//**
 * \file   bit_manip.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#pragma once
#include <algorithm>
#include <bit>       // std::popcount (C++20)
#include <cstdint>
#include <utility>
#include <vector>
#include "dsa_framework/framework.h"

namespace BitManip {

// =============================================================================
//  Standalone bit-manipulation utilities
// =============================================================================

/**
 * count_bits_kernighan  –  Brian Kernighan's trick, O(k) where k = set bits.
 * Each iteration clears the lowest set bit: n &= (n-1).
 * Useful when k is small (sparse bit patterns).
 */
inline int count_bits_kernighan(uint32_t n)
{
    int c = 0;
    while (n) { n &= n - 1; ++c; }
    return c;
}

/** count_bits_popcount  –  C++20 hardware instruction, O(1). */
inline int count_bits_popcount(uint32_t n) { return std::popcount(n); }

/**
 * is_power_of_two  –  true iff n > 0 and exactly one bit is set.
 * The trick: a power of two in binary is 10...0.  Subtracting 1 flips all
 * lower bits: 01...1.  ANDing gives 0.
 */
inline bool is_power_of_two(uint32_t n) { return n > 0 && (n & (n - 1)) == 0; }

/**
 * next_power_of_two  –  smallest power of two >= n.
 * OR-propagates the highest set bit rightward using right-shifts, then +1.
 */
inline uint32_t next_power_of_two(uint32_t n)
{
    if (n == 0) return 1;
    --n;
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16;
    return n + 1;
}

/**
 * reverse_bits  –  reverse all 32 bits using a divide-and-conquer swap.
 * Each stage swaps adjacent groups of increasing width (1, 2, 4, 8, 16 bits).
 */
inline uint32_t reverse_bits(uint32_t n)
{
    n = ((n >> 1)  & 0x55555555u) | ((n & 0x55555555u) << 1);
    n = ((n >> 2)  & 0x33333333u) | ((n & 0x33333333u) << 2);
    n = ((n >> 4)  & 0x0F0F0F0Fu) | ((n & 0x0F0F0F0Fu) << 4);
    n = ((n >> 8)  & 0x00FF00FFu) | ((n & 0x00FF00FFu) << 8);
    n = (n >> 16)                 | (n                 << 16);
    return n;
}

/**
 * single_number_xor  –  XOR trick to find the one element that appears once
 * when all others appear exactly twice.
 *
 * x ^ x == 0  (identical values cancel)
 * x ^ 0 == x  (XOR with zero is identity)
 * XOR is commutative and associative → order doesn't matter.
 * O(n) time, O(1) space.
 */
inline int single_number_xor(const std::vector<int>& arr)
{
    int result = 0;
    for (int x : arr) result ^= x;
    return result;
}

/**
 * two_single_numbers  –  when exactly two elements appear once (all others
 * appear twice), find both using XOR + bit-partitioning.
 *
 * Step 1: XOR all → xor_all = a ^ b (the two unique values).
 * Step 2: any set bit in xor_all differs between a and b.  Pick the lowest.
 * Step 3: partition array on that bit → a ends up in one group, b in the other.
 */
inline std::pair<int, int> two_single_numbers(const std::vector<int>& arr)
{
    int xor_all = 0;
    for (int x : arr) xor_all ^= x;
    int diff_bit = xor_all & (-xor_all);   // isolate lowest set bit
    int a = 0, b = 0;
    for (int x : arr)
    {
        if (x & diff_bit) a ^= x;
        else              b ^= x;
    }
    return {a, b};
}

// =============================================================================
//  Algorithm wrappers
// =============================================================================

// --- Popcount comparison ---
struct PopcountInput  { std::vector<uint32_t> values; };
using  PopcountOutput = std::vector<int>;

/**
 * @brief Batch popcount using Brian Kernighan's bit-clearing trick.
 *
 * @details
 * Applies count_bits_kernighan() to each value in the input vector.
 * Each call runs in O(k) where k is the number of set bits — making this
 * method particularly fast for sparse bit patterns (few 1s set).
 *
 * The trick: `n &= n - 1` clears the lowest set bit in one operation, so
 * the loop iterates exactly as many times as there are set bits.
 *
 * @par Complexity   O(k) per value, where k = popcount(value).
 * @see count_bits_kernighan
 */
struct PopcountKernighan : Algorithm<PopcountInput, PopcountOutput>
{
    PopcountOutput run(const PopcountInput& in) override
    {
        PopcountOutput out;
        out.reserve(in.values.size());
        for (uint32_t v : in.values) out.push_back(count_bits_kernighan(v));
        return out;
    }
    std::string name()       const override { return "Kernighan"; }
    std::string complexity() const override { return "O(k) per value, k=set bits"; }
};

/**
 * @brief Batch popcount using the C++20 std::popcount hardware instruction.
 *
 * @details
 * Applies count_bits_popcount() (which calls std::popcount) to each value.
 * On x86-64 this compiles to a single POPCNT instruction — O(1) regardless
 * of the number of set bits, and typically 1 clock cycle.
 *
 * Use as the performance baseline against Kernighan for dense bit patterns.
 *
 * @par Complexity   O(1) per value (hardware instruction).
 * @see count_bits_popcount
 */
struct PopcountStd : Algorithm<PopcountInput, PopcountOutput>
{
    PopcountOutput run(const PopcountInput& in) override
    {
        PopcountOutput out;
        out.reserve(in.values.size());
        for (uint32_t v : in.values) out.push_back(count_bits_popcount(v));
        return out;
    }
    std::string name()       const override { return "std::popcount"; }
    std::string complexity() const override { return "O(1) hardware instruction"; }
};

// --- Single Number ---
struct SingleInput  { std::vector<int> arr; };
using  SingleOutput = int;

/**
 * @brief Find the one element that appears once when all others appear twice — XOR trick.
 *
 * @details
 * XORs all elements together.  The key identities:
 *   x ^ x = 0  (duplicates cancel).
 *   x ^ 0 = x  (identity).
 *   XOR is commutative and associative (order does not matter).
 *
 * After XORing the full array, all paired elements have cancelled to 0,
 * leaving only the single-occurrence element.
 *
 * @par Complexity   O(n) time, O(1) space.
 * @see single_number_xor
 */
struct SingleNumberXOR : Algorithm<SingleInput, SingleOutput>
{
    SingleOutput run(const SingleInput& in) override
    {
        return single_number_xor(in.arr);
    }
    std::string name()       const override { return "XOR trick"; }
    std::string complexity() const override { return "O(n) time, O(1) space"; }
};

/**
 * @brief Find the single-occurrence element — sort-and-scan baseline.
 *
 * @details
 * Sorts the array, then walks through pairs (i, i+1).  In a sorted array,
 * any element appearing twice will sit adjacent to itself, so the first
 * pair where arr[i] ≠ arr[i+1] reveals the unique element at arr[i].
 * If no such pair is found, the unique element is the last one.
 *
 * Simpler to reason about than the XOR approach; used to validate it in tests.
 *
 * @par Complexity   O(n log n) time, O(n) space (copy for sorting).
 * @see SingleNumberXOR for the O(n)/O(1) alternative.
 */
struct SingleNumberSort : Algorithm<SingleInput, SingleOutput>
{
    SingleOutput run(const SingleInput& in) override
    {
        auto arr = in.arr;
        std::sort(arr.begin(), arr.end());
        for (int i = 0; i + 1 < (int)arr.size(); i += 2)
            if (arr[i] != arr[i + 1]) return arr[i];
        return arr.back();
    }
    std::string name()       const override { return "SortScan"; }
    std::string complexity() const override { return "O(n log n) time, O(n) space"; }
};

}  // namespace BitManip
