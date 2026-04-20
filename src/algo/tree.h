/*****************************************************************//**
 * \file   tree.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <algorithm>
#include <set>
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  Tree namespace
// -----------------------------------------------------------------------------
namespace Tree
{

using Vec = std::vector<int>;

// =============================================================================
//  BST  -  Binary Search Tree
// =============================================================================
/**
 * A Binary Search Tree (BST) maintains the invariant at every node:
 *   all keys in left subtree  <  node key  <=  all keys in right subtree
 *
 * This gives O(log n) average operations, but degrades to O(n) on sorted input
 * because the tree becomes a linked list.  See AVL below for the fix.
 *
 * Complexities (average / worst)
 *   insert   O(log n) / O(n)
 *   search   O(log n) / O(n)
 *   remove   O(log n) / O(n)
 *   inorder  O(n)     (always)
 *   height   O(n)     (always, must traverse)
 *
 * Duplicates go into the right subtree (standard multiset behaviour).
 */
class BST
{
	struct Node
	{
		int   val;
		Node* left  = nullptr;
		Node* right = nullptr;
		explicit Node(int v) : val(v) {}
	};

	Node* root_ = nullptr;
	int   size_ = 0;

	Node* _insert(Node* n, int val)
	{
		if (!n) { ++size_; return new Node(val); }
		if (val < n->val) n->left  = _insert(n->left,  val);
		else              n->right = _insert(n->right, val);  // duplicates go right
		return n;
	}

	bool _search(Node* n, int val) const
	{
		if (!n) return false;
		if (val == n->val) return true;
		return val < n->val ? _search(n->left, val) : _search(n->right, val);
	}

	Node* _min(Node* n) const { while (n->left) n = n->left; return n; }

	Node* _remove(Node* n, int val)
	{
		if (!n) return nullptr;
		if      (val < n->val) n->left  = _remove(n->left,  val);
		else if (val > n->val) n->right = _remove(n->right, val);
		else
		{
			--size_;
			if (!n->left)  { Node* r = n->right; delete n; return r; }
			if (!n->right) { Node* l = n->left;  delete n; return l; }
			// Two children: replace with in-order successor, then delete it
			Node* succ = _min(n->right);
			n->val = succ->val;
			++size_;  // _remove below will decrement again
			n->right = _remove(n->right, succ->val);
		}
		return n;
	}

	void _inorder(Node* n, Vec& out) const
	{
		if (!n) return;
		_inorder(n->left, out);
		out.push_back(n->val);
		_inorder(n->right, out);
	}

	int _height(Node* n) const
	{
		if (!n) return 0;
		return 1 + std::max(_height(n->left), _height(n->right));
	}

	void _destroy(Node* n) { if (!n) return; _destroy(n->left); _destroy(n->right); delete n; }

   public:
	BST() = default;
	~BST() { _destroy(root_); }
	BST(const BST&)            = delete;
	BST& operator=(const BST&) = delete;

	void insert(int val)       { root_ = _insert(root_, val); }
	bool search(int val) const { return _search(root_, val); }
	void remove(int val)       { root_ = _remove(root_, val); }
	Vec  inorder() const       { Vec v; _inorder(root_, v); return v; }
	int  height()  const       { return _height(root_); }
	int  size()    const       { return size_; }
	bool empty()   const       { return size_ == 0; }
};

// =============================================================================
//  AVL  -  Self-balancing BST (Adelson-Velsky & Landis, 1962)
// =============================================================================
/**
 * An AVL tree adds a height-balance invariant on top of the BST invariant:
 *   |height(left subtree) − height(right subtree)| ≤ 1  at every node
 *
 * After each insert/remove, the tree walks back up the path and applies at
 * most O(log n) rotations to restore the invariant.
 *
 * Four rotation cases triggered by balance factor (bf = height_L − height_R):
 *   bf =  2, bf(left)  ≥ 0 → LL case → single right rotation
 *   bf =  2, bf(left)  < 0 → LR case → left rotate child, then right rotate node
 *   bf = -2, bf(right) ≤ 0 → RR case → single left rotation
 *   bf = -2, bf(right) > 0 → RL case → right rotate child, then left rotate node
 *
 * Complexities (guaranteed worst-case, unlike plain BST)
 *   insert   O(log n)
 *   search   O(log n)
 *   remove   O(log n)
 *   inorder  O(n)
 *
 * Height bound: h ≤ 1.44 * log2(n+2) − 0.328
 *
 * When to prefer over BST: whenever input may be sorted/nearly-sorted
 * (pathological for plain BST, transparent for AVL).
 * std::set / std::map use red-black trees (slightly fewer rotations on insert,
 * AVL is slightly faster on lookup-heavy workloads).
 *
 * Duplicates go right (same as BST above).
 */
class AVL
{
	struct Node
	{
		int   val, height = 1;
		Node* left  = nullptr;
		Node* right = nullptr;
		explicit Node(int v) : val(v) {}
	};

	Node* root_ = nullptr;
	int   size_ = 0;

	int _h(Node* n)  const { return n ? n->height : 0; }
	int _bf(Node* n) const { return n ? _h(n->left) - _h(n->right) : 0; }
	void _update(Node* n)  { n->height = 1 + std::max(_h(n->left), _h(n->right)); }

	//   y                x
	//  / \    right    / \
	// x   C   ────>  A   y
	// / \                / \
	//A   B              B   C
	Node* _rot_right(Node* y)
	{
		Node* x = y->left, *B = x->right;
		x->right = y; y->left = B;
		_update(y); _update(x);
		return x;
	}

	// mirror of _rot_right
	Node* _rot_left(Node* x)
	{
		Node* y = x->right, *B = y->left;
		y->left = x; x->right = B;
		_update(x); _update(y);
		return y;
	}

	Node* _balance(Node* n)
	{
		_update(n);
		int bf = _bf(n);
		if (bf >  1) { if (_bf(n->left)  < 0) n->left  = _rot_left(n->left);   return _rot_right(n); }
		if (bf < -1) { if (_bf(n->right) > 0) n->right = _rot_right(n->right); return _rot_left(n);  }
		return n;
	}

	Node* _insert(Node* n, int val)
	{
		if (!n) { ++size_; return new Node(val); }
		if (val < n->val) n->left  = _insert(n->left,  val);
		else              n->right = _insert(n->right, val);  // duplicates go right
		return _balance(n);
	}

	bool _search(Node* n, int val) const
	{
		if (!n) return false;
		if (val == n->val) return true;
		return val < n->val ? _search(n->left, val) : _search(n->right, val);
	}

	Node* _min(Node* n) const { while (n->left) n = n->left; return n; }

	Node* _remove(Node* n, int val)
	{
		if (!n) return nullptr;
		if      (val < n->val) n->left  = _remove(n->left,  val);
		else if (val > n->val) n->right = _remove(n->right, val);
		else
		{
			--size_;
			if (!n->left || !n->right) { Node* c = n->left ? n->left : n->right; delete n; return c; }
			Node* succ = _min(n->right);
			n->val = succ->val;
			++size_;
			n->right = _remove(n->right, succ->val);
		}
		return _balance(n);
	}

	void _inorder(Node* n, Vec& out) const
	{
		if (!n) return;
		_inorder(n->left, out);
		out.push_back(n->val);
		_inorder(n->right, out);
	}

	void _destroy(Node* n) { if (!n) return; _destroy(n->left); _destroy(n->right); delete n; }

   public:
	AVL() = default;
	~AVL() { _destroy(root_); }
	AVL(const AVL&)            = delete;
	AVL& operator=(const AVL&) = delete;

	void insert(int val)       { root_ = _insert(root_, val); }
	bool search(int val) const { return _search(root_, val); }
	void remove(int val)       { root_ = _remove(root_, val); }
	Vec  inorder() const       { Vec v; _inorder(root_, v); return v; }
	int  height()  const       { return _h(root_); }
	int  size()    const       { return size_; }
	bool empty()   const       { return size_ == 0; }
};

// =============================================================================
//  Algorithm wrappers
// =============================================================================

// -- TreeSearch: build tree then search for a target --------------------------
/**
 * Compares BST (O(log n) avg, O(n) worst) vs AVL (O(log n) always) vs
 * std::set (red-black tree, O(log n) always) for a search after bulk insert.
 *
 * The interesting case: insert sorted data → BST degenerates to a linked list
 * (height n), while AVL stays at height ~1.44 log n.
 */
struct TreeSearchInput { Vec vals; int target; };

/**
 * @brief Build a BST from vals, then search for target.
 *
 * @details
 * Demonstrates the average-case O(log n) / worst-case O(n) behaviour of an
 * unbalanced BST.  On sorted input the tree degenerates to a linked list
 * (height = n), making every search O(n).
 *
 * @par Complexity   Build O(n log n) avg / O(n²) worst.   Search O(log n) avg / O(n) worst.
 */
class BSTSearch : public Algorithm<TreeSearchInput, bool>
{
   public:
	bool run(const TreeSearchInput& in) override
	{
		BST t;
		for (int v : in.vals) t.insert(v);
		return t.search(in.target);
	}
	std::string name()        const override { return "BSTSearch"; }
	std::string description() const override { return "O(log n) avg, O(n) worst (unbalanced)"; }
	std::string complexity()  const override { return "O(log n) avg, O(n) worst"; }
};

/**
 * @brief Build an AVL tree from vals, then search for target.
 *
 * @details
 * The AVL height invariant guarantees O(log n) search even on pathological
 * inputs like sorted sequences.  Compare against BSTSearch on sorted data
 * to see the O(n) vs O(log n) contrast.
 *
 * @par Complexity   Build O(n log n) always.   Search O(log n) always.
 */
class AVLSearch : public Algorithm<TreeSearchInput, bool>
{
   public:
	bool run(const TreeSearchInput& in) override
	{
		AVL t;
		for (int v : in.vals) t.insert(v);
		return t.search(in.target);
	}
	std::string name()        const override { return "AVLSearch"; }
	std::string description() const override { return "O(log n) guaranteed (balanced)"; }
	std::string complexity()  const override { return "O(log n) always"; }
};

/**
 * @brief Build a std::set from vals, then search for target.
 *
 * @details
 * std::set is backed by a red-black tree — O(log n) operations guaranteed,
 * similar to AVL but with at most 2 rotations per insert (AVL can do O(log n)
 * rotations on insert but O(1) amortised).  Red-black trees have slightly
 * better insert/delete performance; AVL trees have slightly better lookup
 * performance (stricter balance → shallower tree).
 *
 * Used as the standard-library reference baseline.
 *
 * @par Complexity   Build O(n log n).   Search O(log n).
 */
class StdSetSearch : public Algorithm<TreeSearchInput, bool>
{
   public:
	bool run(const TreeSearchInput& in) override
	{
		std::set<int> s(in.vals.begin(), in.vals.end());
		return s.count(in.target) > 0;
	}
	std::string name()        const override { return "std::set search"; }
	std::string description() const override { return "O(log n) red-black tree (STL)"; }
};

// -- TreeSort: inorder traversal of a BST/AVL yields sorted output ------------
/**
 * Insert n elements then do inorder DFS — this is O(n log n) avg for BST,
 * O(n log n) guaranteed for AVL.
 *
 * Demonstrates why AVL is superior for sorted/nearly-sorted input:
 *   - BST on sorted input: inserts in O(n²) total, height = n
 *   - AVL on sorted input: inserts in O(n log n) total, height = O(log n)
 *
 * Compare vs std::sort to see the constant-factor overhead of pointer chasing.
 */
class BSTSort : public Algorithm<Vec, Vec>
{
   public:
	Vec run(const Vec& input) override
	{
		BST t;
		for (int v : input) t.insert(v);
		return t.inorder();
	}
	std::string name()        const override { return "BSTSort"; }
	std::string description() const override { return "O(n log n) avg via BST inorder"; }
	std::string complexity()  const override { return "O(n log n) avg, O(n^2) on sorted input"; }
};

/**
 * @brief Sort by inserting into an AVL tree and collecting inorder output.
 *
 * @details
 * Guaranteed O(n log n) even on sorted input, unlike BSTSort.
 * Demonstrates the benefit of self-balancing: the AVL height invariant
 * keeps every insert at O(log n) regardless of insertion order.
 *
 * Higher constant factor than std::sort (pointer chasing, per-node heap
 * allocation) but useful to illustrate tree-based sorting correctness.
 *
 * @par Complexity   O(n log n) always.   Space O(n) (n nodes).
 */
class AVLSort : public Algorithm<Vec, Vec>
{
   public:
	Vec run(const Vec& input) override
	{
		AVL t;
		for (int v : input) t.insert(v);
		return t.inorder();
	}
	std::string name()        const override { return "AVLSort"; }
	std::string description() const override { return "O(n log n) guaranteed via AVL inorder"; }
	std::string complexity()  const override { return "O(n log n) always"; }
};

}  // namespace Tree
