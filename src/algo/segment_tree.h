#pragma once
#include <climits>
#include <functional>
#include <vector>
#include "dsa_framework/framework.h"

namespace SegTree {

// =============================================================================
//  SegmentTree<T, Combine>  –  generic segment tree
// =============================================================================
/**
 * A segment tree splits an array into a binary tree of ranges.
 * Each internal node stores the combined value (sum / min / max / gcd …) of
 * its range.  Leaf nodes store individual array elements.
 *
 * Storage: 1-indexed array of size 4*n.
 *   Node 1   = root (covers [0, n-1])
 *   Node 2i  = left  child of i  (covers [l, mid])
 *   Node 2i+1= right child of i  (covers [mid+1, r])
 *
 * Complexities:
 *   build(arr)          O(n)
 *   query(l, r)         O(log n)  — combine results from O(log n) nodes
 *   update(idx, val)    O(log n)  — update leaf + O(log n) ancestors
 *
 * Parameterisation:
 *   Combine = std::plus<int>   → range sum   (identity = 0)
 *   Combine = std::min lambda  → range min   (identity = INT_MAX)
 *   Combine = std::max lambda  → range max   (identity = INT_MIN)
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
