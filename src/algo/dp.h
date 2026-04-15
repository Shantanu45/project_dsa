#pragma once
#include <algorithm>
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

}  // namespace DP