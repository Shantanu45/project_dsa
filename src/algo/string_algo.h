#pragma once
#include <string>
#include <vector>
#include "dsa_framework/framework.h"

namespace StringAlgo {

// =============================================================================
//  KMP  –  Knuth-Morris-Pratt  O(n + m)
// =============================================================================
/**
 * Two-phase algorithm:
 *
 * 1. Build the failure function f[i]:
 *    f[i] = length of the longest proper prefix of pat[0..i] that is also
 *    a suffix.  Tells us how far to "fall back" in the pattern on mismatch
 *    without rescanning text characters.
 *
 * 2. Search:
 *    Slide pattern over text.  On mismatch at pat[j], jump to pat[f[j-1]]
 *    instead of restarting from pat[0].
 *
 * Total: O(n + m) — each text and pattern character is visited at most twice.
 */
inline std::vector<int> kmp_failure(const std::string& pat)
{
    int m = (int)pat.size();
    std::vector<int> f(m, 0);
    for (int i = 1, j = 0; i < m; ++i)
    {
        while (j > 0 && pat[i] != pat[j]) j = f[j - 1];
        if (pat[i] == pat[j]) ++j;
        f[i] = j;
    }
    return f;
}

inline std::vector<int> kmp_search(const std::string& text, const std::string& pat)
{
    std::vector<int> pos;
    if (pat.empty()) return pos;
    auto f = kmp_failure(pat);
    int  n = (int)text.size(), m = (int)pat.size();
    for (int i = 0, j = 0; i < n; ++i)
    {
        while (j > 0 && text[i] != pat[j]) j = f[j - 1];
        if (text[i] == pat[j]) ++j;
        if (j == m) { pos.push_back(i - m + 1); j = f[m - 1]; }
    }
    return pos;
}

// =============================================================================
//  Z-algorithm  –  O(n + m)
// =============================================================================
/**
 * Build the Z-array of a string s:
 *   Z[i] = length of the longest substring starting at s[i] that is also
 *          a prefix of s.  (Z[0] is defined as |s|.)
 *
 * Search trick: concatenate pat + '$' + text.  Any position i (in the text
 * portion) where Z[i] == |pat| is a match start.
 *
 * The '$' sentinel (not in the alphabet) ensures the Z-values in the text
 * portion never exceed |pat|.
 */
inline std::vector<int> z_array(const std::string& s)
{
    int n = (int)s.size();
    std::vector<int> z(n, 0);
    z[0] = n;
    for (int i = 1, l = 0, r = 0; i < n; ++i)
    {
        if (i < r) z[i] = std::min(r - i, z[i - l]);
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) ++z[i];
        if (i + z[i] > r) { l = i; r = i + z[i]; }
    }
    return z;
}

inline std::vector<int> z_search(const std::string& text, const std::string& pat)
{
    std::vector<int> pos;
    if (pat.empty()) return pos;
    std::string combined = pat + "$" + text;
    auto z = z_array(combined);
    int  m = (int)pat.size();
    for (int i = m + 1; i < (int)combined.size(); ++i)
        if (z[i] == m) pos.push_back(i - m - 1);
    return pos;
}

// =============================================================================
//  Naive search  –  O(n * m)
// =============================================================================
inline std::vector<int> naive_search(const std::string& text, const std::string& pat)
{
    std::vector<int> pos;
    if (pat.empty()) return pos;
    int n = (int)text.size(), m = (int)pat.size();
    for (int i = 0; i <= n - m; ++i)
        if (text.compare(i, m, pat) == 0) pos.push_back(i);
    return pos;
}

// =============================================================================
//  Algorithm wrappers
// =============================================================================
struct SearchInput
{
    std::string text;
    std::string pattern;
};
using SearchOutput = std::vector<int>;   // 0-indexed match positions

struct KMPSearch : Algorithm<SearchInput, SearchOutput>
{
    SearchOutput run(const SearchInput& in) override
    {
        return kmp_search(in.text, in.pattern);
    }
    std::string name()       const override { return "KMP"; }
    std::string complexity() const override { return "O(n+m)"; }
};

struct ZSearch : Algorithm<SearchInput, SearchOutput>
{
    SearchOutput run(const SearchInput& in) override
    {
        return z_search(in.text, in.pattern);
    }
    std::string name()       const override { return "Z-Algorithm"; }
    std::string complexity() const override { return "O(n+m)"; }
};

struct NaiveSearch : Algorithm<SearchInput, SearchOutput>
{
    SearchOutput run(const SearchInput& in) override
    {
        return naive_search(in.text, in.pattern);
    }
    std::string name()       const override { return "NaiveSearch"; }
    std::string complexity() const override { return "O(n*m)"; }
};

}  // namespace StringAlgo
