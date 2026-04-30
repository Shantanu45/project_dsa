/*****************************************************************//**
 * \file   dp.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#pragma once
#include <algorithm>
#include <climits>
#include <string>
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  DP namespace
// -----------------------------------------------------------------------------
namespace DP
{

// -- 0/1 Knapsack --------------------------------------------------------------
/**
 * @brief 0/1 Knapsack: maximise total value subject to a weight capacity.
 *
 * @details
 * Given n items each with a weight and a value, and a knapsack of capacity W,
 * find the maximum value achievable by selecting a subset of items whose total
 * weight does not exceed W.  Each item can be taken at most once (0/1).
 *
 * @par Algorithm (bottom-up DP table)
 *   dp[i][c] = maximum value using items 0..i-1 with capacity c.
 *
 *   Recurrence:
 *     dp[i][c] = dp[i-1][c]                                      (skip item i)
 *     dp[i][c] = max(dp[i][c], dp[i-1][c - w[i]] + v[i])        (take item i, if w[i] <= c)
 *
 *   Base case: dp[0][*] = 0 (no items -> zero value).
 *   Answer: dp[n][W].
 *
 * @par Complexity
 *   Time  O(n × W) — fills an (n+1) × (W+1) table.
 *   Space O(n × W) — can be reduced to O(W) with a rolling 1-D array.
 *
 * @par Classic uses
 *   Resource allocation, cargo loading, portfolio selection, subset-sum variant.
 */
struct KnapsackInput
{
	int capacity;
	std::vector<int> weights;
	std::vector<int> values;
};

class Knapsack : public Algorithm<KnapsackInput, int>
{
   public:
	int run(const KnapsackInput& in) override
	{
		int n = (int)in.weights.size();
		std::vector<std::vector<int>> dp(n + 1, std::vector<int>(in.capacity + 1, 0));
		for (int i = 1; i <= n; ++i)
			for (int c = 0; c <= in.capacity; ++c)
			{
				dp[i][c] = dp[i - 1][c];
				if (in.weights[i - 1] <= c)
					dp[i][c] = std::max(dp[i][c], dp[i - 1][c - in.weights[i - 1]] + in.values[i - 1]);
			}
		return dp[n][in.capacity];
	}
	std::string name() const override { return "Knapsack 0/1"; }
	std::string description() const override { return "O(n*W) DP"; }
};

// -- LCS (Longest Common Subsequence) -----------------------------------------
/**
 * @brief Find the length of the longest common subsequence of two strings.
 *
 * @details
 * A subsequence is obtained by deleting some (or no) characters without
 * changing the order of the remaining characters.  "ACE" is a subsequence of
 * "ABCDE" but "AEC" is not.
 *
 * @par Algorithm (classic 2-D DP)
 *   dp[i][j] = LCS length of a[0..i-1] and b[0..j-1].
 *
 *   Recurrence:
 *     If a[i-1] == b[j-1]:  dp[i][j] = dp[i-1][j-1] + 1   (extend the common subseq)
 *     Else:                  dp[i][j] = max(dp[i-1][j], dp[i][j-1])  (skip one char)
 *
 *   Base case: dp[0][*] = dp[*][0] = 0.
 *   Answer: dp[m][n].
 *
 * @par Complexity
 *   Time  O(m × n) where m = |a|, n = |b|.
 *   Space O(m × n) — reducible to O(min(m,n)) with a two-row rolling array.
 *
 * @par Classic uses
 *   Diff tools (git diff), DNA sequence alignment, spell-checker suggestions,
 *   file patching.  The actual subsequence can be reconstructed via traceback.
 */
struct LCSInput
{
	std::string a, b;
};

class LCS : public Algorithm<LCSInput, int>
{
   public:
	int run(const LCSInput& in) override
	{
		int m = (int)in.a.size(), n = (int)in.b.size();
		std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
		for (int i = 1; i <= m; ++i)
			for (int j = 1; j <= n; ++j)
				dp[i][j] = (in.a[i - 1] == in.b[j - 1]) ? dp[i - 1][j - 1] + 1 : std::max(dp[i - 1][j], dp[i][j - 1]);
		return dp[m][n];
	}
	std::string name() const override { return "LCS"; }
	std::string description() const override { return "O(m*n) DP, longest common subsequence"; }
};

// -- Coin Change (min coins) ---------------------------------------------------
/**
 * @brief Find the minimum number of coins that sum to a target amount.
 *
 * @details
 * Given an infinite supply of coin denominations, find the fewest coins
 * needed to make exactly 'amount'.  This is the unbounded knapsack variant
 * (each coin can be used any number of times).
 *
 * @par Algorithm (1-D bottom-up DP)
 *   dp[a] = minimum coins needed to make amount a.
 *
 *   Recurrence (for each amount a from 1 to W, each coin c):
 *     If c <= a:  dp[a] = min(dp[a], dp[a - c] + 1)
 *
 *   Base case: dp[0] = 0.
 *   Answer: dp[amount], or -1 if dp[amount] was never updated from INF.
 *
 * @par Complexity
 *   Time  O(amount × |coins|).
 *   Space O(amount).
 *
 * @par Classic uses
 *   Cashier change-making, resource scheduling, jump-game variants.
 *   The greedy algorithm (always take the largest coin) fails for arbitrary
 *   denominations (e.g. coins={1,3,4}, amount=6: greedy gives 4+1+1=3 coins,
 *   DP gives 3+3=2 coins).
 */
struct CoinInput
{
	std::vector<int> coins;
	int amount;
};

class CoinChange : public Algorithm<CoinInput, int>
{
   public:
	int run(const CoinInput& in) override
	{
		const int INF = in.amount + 1;
		std::vector<int> dp(in.amount + 1, INF);
		dp[0] = 0;
		for (int i = 1; i <= in.amount; ++i)
			for (int c : in.coins)
				if (c <= i)
					dp[i] = std::min(dp[i], dp[i - c] + 1);
		return dp[in.amount] > in.amount ? -1 : dp[in.amount];
	}
	std::string name() const override { return "CoinChange"; }
	std::string description() const override { return "O(amount * coins) DP, min coins"; }
};

// -- Edit Distance (Levenshtein) -----------------------------------------------
/**
 * @brief Compute the minimum edit distance (Levenshtein distance) between two strings.
 *
 * @details
 * The edit distance is the minimum number of single-character operations
 * (insert, delete, substitute) needed to transform string a into string b.
 *
 * @par Algorithm (2-D DP — same table shape as LCS)
 *   dp[i][j] = edit distance between a[0..i-1] and b[0..j-1].
 *
 *   Recurrence:
 *     If a[i-1] == b[j-1]:  dp[i][j] = dp[i-1][j-1]            (no operation needed)
 *     Else:                  dp[i][j] = 1 + min(
 *                                           dp[i-1][j],          (delete from a)
 *                                           dp[i][j-1],          (insert into a)
 *                                           dp[i-1][j-1] )       (substitute)
 *
 *   Base cases:
 *     dp[i][0] = i  (delete i chars to reach empty string)
 *     dp[0][j] = j  (insert j chars to build b from empty)
 *
 *   Answer: dp[m][n].
 *
 * @par Complexity
 *   Time  O(m × n).
 *   Space O(m × n) — reducible to O(min(m,n)) with two rolling rows.
 *
 * @par Classic uses
 *   Spell-checkers, DNA alignment, fuzzy string matching, version control
 *   diff, autocorrect, plagiarism detection.
 */
class EditDistance : public Algorithm<LCSInput, int>
{
   public:
	int run(const LCSInput& in) override
	{
		int m = (int)in.a.size(), n = (int)in.b.size();
		std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
		for (int i = 0; i <= m; ++i)
			dp[i][0] = i;
		for (int j = 0; j <= n; ++j)
			dp[0][j] = j;
		for (int i = 1; i <= m; ++i)
			for (int j = 1; j <= n; ++j)
				dp[i][j] = (in.a[i - 1] == in.b[j - 1]) ? dp[i - 1][j - 1] : 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
		return dp[m][n];
	}
	std::string name() const override { return "EditDistance"; }
	std::string description() const override { return "O(m*n) Levenshtein distance"; }
};

using Vec = std::vector<int>;

// -- LIS: Longest Increasing Subsequence --------------------------------------
/**
 * @brief Find the length of the longest strictly increasing subsequence.
 *
 * @details
 * Given a sequence, find the length of the longest strictly increasing
 * subsequence (elements need not be contiguous).
 *
 * Two approaches are implemented:
 *
 * @par LISNaive — O(n²) classic DP
 *   dp[i] = length of the LIS ending at index i.
 *
 *   Recurrence:
 *     dp[i] = max(dp[j] + 1) for all j < i where arr[j] < arr[i]
 *     dp[i] = 1 (base case: subseq of just arr[i])
 *
 *   Answer: max over all dp[i].
 *
 * @par LISBinarySearch — O(n log n) patience sorting
 *   Maintains a list 'tails' where tails[k] is the smallest tail element of
 *   all increasing subsequences of length k+1 seen so far.
 *
 *   For each element x:
 *     - Binary-search 'tails' for the first element ≥ x.
 *     - Replace it with x (or append if x > all tails).
 *
 *   The length of 'tails' at the end equals the LIS length.
 *
 *   Why this works: 'tails' is always sorted (invariant), so binary search
 *   applies.  Replacing a tail with a smaller value never invalidates existing
 *   subsequences — it only opens more possibilities for future elements.
 *   Note: 'tails' itself is NOT necessarily an actual LIS (it's a maintenance
 *   structure), but its length is correct.
 *
 * @par Complexity
 *   LISNaive:        O(n²) time,       O(n) space.
 *   LISBinarySearch: O(n log n) time,  O(n) space.
 *
 * @par Classic uses
 *   Longest non-decreasing run in stock prices, patience card game, box
 *   stacking, scheduling with deadline constraints.
 */
class LISNaive : public Algorithm<Vec, int>
{
   public:
	int run(const Vec& in) override
	{
		int n = (int)in.size();
		if (n == 0) return 0;
		Vec dp(n, 1);
		for (int i = 1; i < n; ++i)
			for (int j = 0; j < i; ++j)
				if (in[j] < in[i]) dp[i] = std::max(dp[i], dp[j] + 1);
		return *std::max_element(dp.begin(), dp.end());
	}
	std::string name()        const override { return "LIS Naive"; }
	std::string description() const override { return "O(n^2) classic DP"; }
	std::string complexity()  const override { return "O(n^2) time, O(n) space"; }
};

/**
 * @brief O(n log n) LIS using patience sorting with binary search.
 * @see LISNaive for full algorithm description.
 */
class LISBinarySearch : public Algorithm<Vec, int>
{
   public:
	int run(const Vec& in) override
	{
		Vec tails;  // tails[k] = smallest tail of all LIS of length k+1
		for (int x : in)
		{
			auto it = std::lower_bound(tails.begin(), tails.end(), x);
			if (it == tails.end()) tails.push_back(x);
			else                   *it = x;
		}
		return (int)tails.size();
	}
	std::string name()        const override { return "LIS BinSearch"; }
	std::string description() const override { return "O(n log n) patience sorting"; }
	std::string complexity()  const override { return "O(n log n) time, O(n) space"; }
};

// -- Matrix Chain Multiplication -----------------------------------------------
/**
 * @brief Find the minimum scalar multiplications to multiply a chain of matrices.
 *
 * @details
 * Given n matrices A₁, A₂, …, Aₙ where Aᵢ has dimensions dims[i-1] × dims[i],
 * find the minimum number of scalar multiplications to compute A₁ · A₂ · … · Aₙ.
 *
 * Multiplying (a×b) by (b×c) costs a·b·c scalar multiplications.
 * Matrix multiplication is associative, so parenthesisation matters enormously:
 *   (A·B)·C  vs  A·(B·C) can differ by orders of magnitude in cost.
 *
 * @par Algorithm (interval DP)
 *   dp[i][j] = minimum cost to multiply matrices i through j (0-indexed).
 *
 *   Recurrence (try every split point k between i and j):
 *     dp[i][j] = min over k in [i, j-1] of:
 *                  dp[i][k] + dp[k+1][j] + dims[i] × dims[k+1] × dims[j+1]
 *
 *   Base case: dp[i][i] = 0 (single matrix, no multiplication).
 *   Iteration order: increasing chain length (len = 2, 3, …, n).
 *
 * @par Complexity
 *   Time  O(n³) — three nested loops over chain positions.
 *   Space O(n²) — the DP table.
 *
 * @par Note on "interval DP"
 *   The recurrence iterates over sub-intervals of the chain rather than
 *   single-element prefixes.  This "interval DP" pattern also appears in
 *   optimal BST, palindrome partitioning, and burst balloons.
 */
struct MCMInput { std::vector<int> dims; };  // n matrices -> n+1 dimensions

class MatrixChainMult : public Algorithm<MCMInput, long long>
{
   public:
	long long run(const MCMInput& in) override
	{
		int n = (int)in.dims.size() - 1;  // number of matrices
		if (n <= 1) return 0;
		// dp[i][j] = min scalar multiplications for matrices i..j  (0-indexed)
		std::vector<std::vector<long long>> dp(n, std::vector<long long>(n, 0));
		for (int len = 2; len <= n; ++len)
		{
			for (int i = 0; i <= n - len; ++i)
			{
				int j = i + len - 1;
				dp[i][j] = LLONG_MAX;
				for (int k = i; k < j; ++k)
				{
					long long cost = dp[i][k] + dp[k + 1][j]
					               + (long long)in.dims[i] * in.dims[k + 1] * in.dims[j + 1];
					dp[i][j] = std::min(dp[i][j], cost);
				}
			}
		}
		return dp[0][n - 1];
	}
	std::string name()        const override { return "MatrixChainMult"; }
	std::string description() const override { return "O(n^3) interval DP, min multiplications"; }
	std::string complexity()  const override { return "O(n^3) time, O(n^2) space"; }
};

}  // namespace DP
