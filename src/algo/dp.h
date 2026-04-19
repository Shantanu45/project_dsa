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
 * Given a sequence, find the length of the longest strictly increasing
 * subsequence (elements need not be contiguous).
 *
 * Two approaches:
 *
 *  Naive DP — O(n²):
 *    dp[i] = length of LIS ending at index i
 *    dp[i] = max(dp[j] + 1) for all j < i where arr[j] < arr[i]
 *
 *  Patience sorting — O(n log n):
 *    Maintain a list 'tails' where tails[k] is the smallest tail element of
 *    all increasing subsequences of length k+1 seen so far.
 *    For each element x: binary-search for the first tail >= x and replace it
 *    with x (or append if x > all tails).
 *    The length of 'tails' at the end equals the LIS length.
 *
 *    Why does this work? tails is always sorted, so binary search applies.
 *    Replacing a tail with a smaller value never invalidates existing
 *    subsequences — it only creates more possibilities for future elements.
 *
 * Classic uses: longest non-decreasing run in stock prices, patience card game,
 *   box stacking, scheduling problems.
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
 * Given n matrices A₁, A₂, …, Aₙ where Aᵢ has dimensions dims[i-1] × dims[i],
 * find the minimum number of scalar multiplications needed to compute the
 * product A₁ · A₂ · … · Aₙ.
 *
 * Multiplying an (a×b) matrix by a (b×c) matrix costs a·b·c multiplications.
 * Matrix multiplication is associative, so parenthesisation matters:
 *   (A·B)·C  vs  A·(B·C) can differ dramatically in cost.
 *
 * Interval DP:
 *   dp[i][j] = min cost to multiply matrices i through j
 *   dp[i][j] = min over k in [i, j-1] of:
 *               dp[i][k] + dp[k+1][j] + dims[i] * dims[k+1] * dims[j+1]
 *
 * Complexities  O(n³) time,  O(n²) space
 *
 * This demonstrates "interval DP" — a different recurrence shape from the
 * linear (knapsack, LIS) and string (LCS, edit distance) DPs.
 */
struct MCMInput { std::vector<int> dims; };  // n matrices → n+1 dimensions

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