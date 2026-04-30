/*****************************************************************//**
 * \file   segment_tree.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <climits>
#include <functional>
#include <vector>
#include "dsa_framework/framework.h"

namespace SegTree {

// =============================================================================
//  SegmentTree<T, Combine>  –  generic segment tree
// =============================================================================
/**
 * @brief Generic segment tree for range queries and point updates.
 *
 * @details
 * A segment tree splits an array into a binary tree of ranges.  Each internal
 * node stores the combined result (sum, min, max, gcd, …) of all elements in
 * its range.  Leaf nodes store individual array elements.
 *
 * @par Storage layout (1-indexed)
 *   Node 1    = root (covers [0, n-1])
 *   Node 2i   = left  child of i (covers [l, mid])
 *   Node 2i+1 = right child of i (covers [mid+1, r])
 *   Array size: 4 × n (safe upper bound — the tree is a complete binary tree
 *   padded to a power of two).
 *
 * @par Build — O(n)
 *   Recursively set each leaf to arr[i], then propagate up:
 *     tree[node] = combine(tree[2*node], tree[2*node+1])
 *
 * @par Query(l, r) — O(log n)
 *   Walk the tree.  If the current node's range is fully inside [l, r],
 *   return tree[node].  If fully outside, return the identity element.
 *   Otherwise recurse into both children and combine their results.
 *   At most O(4 * log n) nodes are visited per query.
 *
 * @par Update(idx, val) — O(log n)
 *   Update the leaf at idx, then recompute all O(log n) ancestors on the path
 *   back to the root.
 *
 * @par Parameterisation
 *   Combine = std::plus<int>    -> range sum   (identity = 0)
 *   Combine = std::min lambda   -> range min   (identity = INT_MAX)
 *   Combine = std::max lambda   -> range max   (identity = INT_MIN)
 *   Any associative, commutative binary operation works.
 *
 * @par Complexity
 *   build    O(n).
 *   query    O(log n).
 *   update   O(log n).
 *   Space    O(n).
 *
 * @par Classic uses
 *   Range sum / min / max queries with point updates; Fenwick tree is lighter
 *   for pure range-sum but cannot handle arbitrary operations.
 *   Segment tree with lazy propagation (not shown here) handles range updates.
 */
template <typename T, typename Combine = std::plus<T>>
class SegmentTree
{
    int            n_;
    T              identity_;
    Combine        combine_;
    std::vector<T> tree_;

    void build(const std::vector<T>& arr, int node, int l, int r)
    {
        if (l == r) { tree_[node] = arr[l]; return; }
        int mid = (l + r) / 2;
        build(arr, 2 * node,     l,     mid);
        build(arr, 2 * node + 1, mid + 1, r);
        tree_[node] = combine_(tree_[2 * node], tree_[2 * node + 1]);
    }

    void update(int node, int l, int r, int idx, T val)
    {
        if (l == r) { tree_[node] = val; return; }
        int mid = (l + r) / 2;
        if (idx <= mid) update(2 * node,     l,     mid, idx, val);
        else            update(2 * node + 1, mid + 1, r, idx, val);
        tree_[node] = combine_(tree_[2 * node], tree_[2 * node + 1]);
    }

    T query(int node, int l, int r, int ql, int qr) const
    {
        if (ql <= l && r <= qr) return tree_[node];
        if (qr < l || r < ql)  return identity_;
        int mid = (l + r) / 2;
        return combine_(query(2 * node,     l,     mid, ql, qr),
                        query(2 * node + 1, mid + 1, r, ql, qr));
    }

   public:
    SegmentTree() : n_(0), identity_() {}
    explicit SegmentTree(T identity, Combine combine = Combine{})
        : n_(0), identity_(identity), combine_(combine)
    {}

    void build(const std::vector<T>& arr)
    {
        n_ = (int)arr.size();
        tree_.assign(4 * n_, identity_);
        if (n_ > 0) build(arr, 1, 0, n_ - 1);
    }

    void update(int idx, T val)      { update(1, 0, n_ - 1, idx, val); }
    T    query(int l, int r)   const { return query(1, 0, n_ - 1, l, r); }
    int  size()                const { return n_; }
};

// =============================================================================
//  Input / Output types
// =============================================================================
struct RangeQuery { int l, r; };
struct UpdateOp   { int idx, val; };

struct RangeInput
{
    std::vector<int>         arr;
    std::vector<RangeQuery>  queries;
    std::vector<UpdateOp>    updates;   // applied before all queries
};
using RangeOutput = std::vector<int>;

// =============================================================================
//  Range Sum
// =============================================================================
/**
 * @brief Range sum query with point updates using a segment tree.
 *
 * @details
 * Answers queries of the form "what is the sum of arr[l..r]?" after applying
 * a batch of point updates.  Uses SegmentTree<int, std::plus<int>> with
 * identity element 0.
 *
 * @par Complexity
 *   Build O(n), each update O(log n), each query O(log n).
 *   Naive baseline is O(n) per query — segment tree wins when queries ≫ 1.
 */
struct RangeSumTree : Algorithm<RangeInput, RangeOutput>
{
    RangeOutput run(const RangeInput& in) override
    {
        SegmentTree<int> st(0);
        st.build(in.arr);
        for (auto& u : in.updates) st.update(u.idx, u.val);
        RangeOutput out;
        for (auto& q : in.queries) out.push_back(st.query(q.l, q.r));
        return out;
    }
    std::string name()       const override { return "RangeSumTree"; }
    std::string complexity() const override { return "O(n) build, O(log n) query/update"; }
};

/**
 * @brief Range sum query — O(n) per query naive baseline.
 *
 * @details
 * Applies updates directly to the array, then answers each query by scanning
 * arr[l..r].  Correct and simple; use to validate the tree implementation.
 */
struct RangeSumNaive : Algorithm<RangeInput, RangeOutput>
{
    RangeOutput run(const RangeInput& in) override
    {
        auto arr = in.arr;
        for (auto& u : in.updates) arr[u.idx] = u.val;
        RangeOutput out;
        for (auto& q : in.queries)
        {
            int s = 0;
            for (int i = q.l; i <= q.r; ++i) s += arr[i];
            out.push_back(s);
        }
        return out;
    }
    std::string name()       const override { return "RangeSumNaive"; }
    std::string complexity() const override { return "O(n) per query"; }
};

// =============================================================================
//  Range Min
// =============================================================================
/**
 * @brief Range minimum query with point updates using a segment tree.
 *
 * @details
 * Uses SegmentTree<int, min-lambda> with identity element INT_MAX.
 * Answers "what is the minimum of arr[l..r]?" after point updates.
 * Equivalent to a sparse table for static arrays, but supports updates.
 *
 * @par Classic uses
 *   Range minimum query (RMQ), lowest common ancestor (via Euler tour),
 *   histogram largest-rectangle (stack approach is O(n) total, but segment
 *   tree handles the online / with-updates variant).
 *
 * @par Complexity
 *   Build O(n), each update/query O(log n).
 */
struct RangeMinTree : Algorithm<RangeInput, RangeOutput>
{
    RangeOutput run(const RangeInput& in) override
    {
        auto cmp = [](int a, int b) { return std::min(a, b); };
        SegmentTree<int, decltype(cmp)> st(INT_MAX, cmp);
        st.build(in.arr);
        for (auto& u : in.updates) st.update(u.idx, u.val);
        RangeOutput out;
        for (auto& q : in.queries) out.push_back(st.query(q.l, q.r));
        return out;
    }
    std::string name()       const override { return "RangeMinTree"; }
    std::string complexity() const override { return "O(n) build, O(log n) query"; }
};

/**
 * @brief Range minimum query — O(n) per query naive baseline.
 */
struct RangeMinNaive : Algorithm<RangeInput, RangeOutput>
{
    RangeOutput run(const RangeInput& in) override
    {
        auto arr = in.arr;
        for (auto& u : in.updates) arr[u.idx] = u.val;
        RangeOutput out;
        for (auto& q : in.queries)
        {
            int m = INT_MAX;
            for (int i = q.l; i <= q.r; ++i) m = std::min(m, arr[i]);
            out.push_back(m);
        }
        return out;
    }
    std::string name()       const override { return "RangeMinNaive"; }
    std::string complexity() const override { return "O(n) per query"; }
};

// =============================================================================
//  Range Max
// =============================================================================
/**
 * @brief Range maximum query with point updates using a segment tree.
 *
 * @details
 * Uses SegmentTree<int, max-lambda> with identity element INT_MIN.
 * Answers "what is the maximum of arr[l..r]?" after point updates.
 *
 * @par Complexity
 *   Build O(n), each update/query O(log n).
 */
struct RangeMaxTree : Algorithm<RangeInput, RangeOutput>
{
    RangeOutput run(const RangeInput& in) override
    {
        auto cmp = [](int a, int b) { return std::max(a, b); };
        SegmentTree<int, decltype(cmp)> st(INT_MIN, cmp);
        st.build(in.arr);
        for (auto& u : in.updates) st.update(u.idx, u.val);
        RangeOutput out;
        for (auto& q : in.queries) out.push_back(st.query(q.l, q.r));
        return out;
    }
    std::string name()       const override { return "RangeMaxTree"; }
    std::string complexity() const override { return "O(n) build, O(log n) query"; }
};

/**
 * @brief Range maximum query — O(n) per query naive baseline.
 */
struct RangeMaxNaive : Algorithm<RangeInput, RangeOutput>
{
    RangeOutput run(const RangeInput& in) override
    {
        auto arr = in.arr;
        for (auto& u : in.updates) arr[u.idx] = u.val;
        RangeOutput out;
        for (auto& q : in.queries)
        {
            int m = INT_MIN;
            for (int i = q.l; i <= q.r; ++i) m = std::max(m, arr[i]);
            out.push_back(m);
        }
        return out;
    }
    std::string name()       const override { return "RangeMaxNaive"; }
    std::string complexity() const override { return "O(n) per query"; }
};

}  // namespace SegTree
