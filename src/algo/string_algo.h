/*****************************************************************//**
 * \file   string_algo.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/
#include <string>
#include <vector>
#include "dsa_framework/framework.h"

namespace StringAlgo {

// =============================================================================
//  KMP  –  Knuth-Morris-Pratt  O(n + m)
// =============================================================================
/**
 * @brief Linear-time exact string search using a precomputed failure function.
 *
 * @details
 * Two-phase algorithm:
 *
 * @par Phase 1 – Build the failure function f[] in O(m)
 *   f[i] = length of the longest proper prefix of pat[0..i] that is also a
 *   suffix of pat[0..i].  This is sometimes called the "partial match" or
 *   "π" table (from the KMP paper).
 *
 *   Example: pat = "ABABC"
 *     f = [0, 0, 1, 2, 0]
 *     f[3]=2 because "AB" is both a prefix and a suffix of "ABAB".
 *
 * @par Phase 2 – Search in O(n)
 *   Maintain a pointer j into the pattern.  For each text character text[i]:
 *     - While j > 0 and text[i] ≠ pat[j], fall back: j = f[j-1].
 *       This is the key KMP insight: we don't need to restart from pat[0]
 *       because the failure function tells us the longest safe overlap.
 *     - If text[i] == pat[j], advance j.
 *     - If j == m, record a match at position i-m+1, then fall back: j = f[m-1].
 *
 * @par Why O(n) and not O(n*m)?
 *   The pointer i into text never moves backward.  Each fall-back in the inner
 *   while-loop decreases j, and j can only increase n times total (once per
 *   text character), so the total number of fall-backs across the entire search
 *   is bounded by n.  The phases therefore run in O(m) + O(n) = O(n+m).
 *
 * @par Complexity
 *   Build: O(m).   Search: O(n).   Total: O(n + m).   Space: O(m).
 *
 * @par Classic uses
 *   Substring search in editors/grep, DNA pattern matching, network intrusion
 *   detection (pattern matching in packet payloads).
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
 * @brief Linear-time string search via the Z-array.
 *
 * @details
 * @par Step 1 – Build the Z-array of string s in O(|s|)
 *   Z[i] = length of the longest substring starting at s[i] that is also a
 *   prefix of s.  (Z[0] is conventionally set to |s|.)
 *
 *   Computed with a "Z-box" [l, r] that tracks the rightmost matching window:
 *     - If i is inside the current Z-box (i < r), initialise Z[i] from the
 *       already-known value Z[i-l] (possible overlap savings).
 *     - Then extend naively while s[Z[i]] == s[i + Z[i]].
 *     - Update the Z-box if the match reaches beyond r.
 *
 *   Each character is visited at most twice (once to extend, once already
 *   inside a box), giving O(|s|) total.
 *
 * @par Step 2 – Search using the concatenation trick
 *   Construct combined = pat + '$' + text.
 *   Any position i in the text portion (i > m) where Z[i] == m is a match
 *   at text position i - m - 1 (subtracting the pattern length and sentinel).
 *
 *   The '$' sentinel character (not in the alphabet) ensures that Z-values
 *   in the text portion are capped at m — they can never "see through" into
 *   the pattern portion — keeping the result correct.
 *
 * @par Complexity
 *   O(n + m) time and space.  Comparable to KMP; Z-algorithm is often simpler
 *   to implement from scratch.
 *
 * @par Classic uses
 *   Same use cases as KMP.  Also useful for finding all periods of a string
 *   and for the "shortest string containing two strings as substrings" problem.
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
/**
 * @brief Brute-force substring search: try every alignment.
 *
 * @details
 * For each starting position i in text (0 ≤ i ≤ n-m), compare pat against
 * text[i..i+m-1] character by character.  Record i on a full match.
 *
 * std::string::compare is used for each window — the compiler typically
 * generates a memcmp-style loop that is SIMD-vectorised, making this approach
 * competitive with KMP/Z for short patterns on modern hardware.
 *
 * @par Complexity
 *   Time  O(n × m) worst case (e.g. text = "aaa…a", pat = "aaa…ab").
 *   Space O(1) extra (beyond the output vector).
 *
 * @par When to use
 *   Short patterns (m ≤ ~8) or small texts where setup overhead of KMP/Z
 *   outweighs the asymptotic saving.  Use as correctness baseline in tests.
 */
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

/**
 * @brief Algorithm wrapper: KMP search.
 * @see kmp_search for the full algorithm description.
 */
struct KMPSearch : Algorithm<SearchInput, SearchOutput>
{
    SearchOutput run(const SearchInput& in) override
    {
        return kmp_search(in.text, in.pattern);
    }
    std::string name()       const override { return "KMP"; }
    std::string complexity() const override { return "O(n+m)"; }
};

/**
 * @brief Algorithm wrapper: Z-algorithm search.
 * @see z_search for the full algorithm description.
 */
struct ZSearch : Algorithm<SearchInput, SearchOutput>
{
    SearchOutput run(const SearchInput& in) override
    {
        return z_search(in.text, in.pattern);
    }
    std::string name()       const override { return "Z-Algorithm"; }
    std::string complexity() const override { return "O(n+m)"; }
};

/**
 * @brief Algorithm wrapper: naive brute-force search.
 * @see naive_search for the full algorithm description.
 */
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
