# Resources & Further Reading {#resources}

Curated references organised by topic. Each section lists the most useful
starting points first.

---

## Algorithms & Data Structures

### Books

| Title | Authors | Notes |
|---|---|---|
| *Introduction to Algorithms* (CLRS), 4th ed. | Cormen, Leiserson, Rivest, Stein | The definitive textbook; rigorous proofs and pseudocode for every algorithm in this project |
| *The Algorithm Design Manual*, 3rd ed. | Skiena | Stronger on intuition and real-world applicability than CLRS; excellent "war stories" |
| *Algorithms*, 4th ed. | Sedgewick & Wayne | Java-based but clear diagrams; companion site has interactive visualisations |
| *Competitive Programmer's Handbook* | Laaksonen | Free PDF; concise coverage of graph algorithms, DP, and data structures used in contests |

### Online References

- **CP-Algorithms** — https://cp-algorithms.com  
  English translation of the famous *e-maxx* site. Deep dives on graph algorithms,
  number theory, strings, and data structures with proofs and C++ code.

- **USACO Guide** — https://usaco.guide  
  Structured curriculum from Bronze to Platinum. Excellent for graph algorithms,
  segment trees, and DP with graded problem sets.

- **Visualgo** — https://visualgo.net  
  Step-through animations for sorting, searching, graphs, trees, heaps, and more.
  Useful when an algorithm's behaviour is not yet intuitive.

- **Big-O Cheat Sheet** — https://www.bigocheatsheet.com  
  Quick-reference for time and space complexities of common data structures and
  sorting algorithms.

---

## Sorting

- **Sorting Algorithm Animations** — https://www.toptal.com/developers/sorting-algorithms  
  Side-by-side visualisations of QuickSort, MergeSort, HeapSort, etc. on different
  input distributions (random, nearly sorted, reversed).

- **Why is QuickSort faster than MergeSort in practice?**  
  https://cs.stackexchange.com/questions/3/why-is-quicksort-better-than-other-sorting-algorithms-in-practice  
  Stack Exchange thread with cache-locality and constant-factor explanations.

---

## Graph Algorithms

- **Graph Theory Playlist** — William Fiset on YouTube  
  Search: *"William Fiset Graph Theory"*  
  ~45-video series covering BFS, DFS, Dijkstra, Bellman-Ford, Union-Find, MST,
  and more with clear animations and Java/Python code.

- **Dijkstra vs Bellman-Ford** — https://cp-algorithms.com/graph/dijkstra.html  
  CP-Algorithms pages for both algorithms with worked examples.

---

## Dynamic Programming

- **DP for Beginners** — LeetCode Discuss  
  Search: *"LeetCode DP for beginners patterns"*  
  Community post cataloguing the core DP patterns (linear, interval, knapsack,
  state machine) with problem links.

- **Atcoder DP Contest** — https://atcoder.jp/contests/dp  
  26 DP problems (A–Z) that systematically cover every pattern: Knapsack, LCS,
  LIS, interval DP, tree DP, digit DP, and more. Free to attempt, editorial
  solutions available online.

- *Knapsack Problems* — Pisinger (1995)  
  Academic survey if you need the full theory behind 0/1, bounded, and unbounded
  variants.

---

## Trees & Tries

- **Ben Langmead's Trie Lecture** — YouTube  
  Search: *"Ben Langmead Trie"*  
  Clear walkthrough of trie construction, search, and prefix counting.

- **AVL Tree Visualisation** — https://www.cs.usfca.edu/~galles/visualization/AVLtree.html  
  Interactive insert/delete showing rotations in real time.

- **Segment Tree Tutorial** — https://cp-algorithms.com/data_structures/segment_tree.html  
  Covers point update, range query, lazy propagation, and persistent variants.

---

## Heaps & Priority Queues

- **Binary Heap** — https://cp-algorithms.com/data_structures/binary_heap.html

- **Why Floyd's heapify is O(n)** — Stack Overflow  
  Search: *"Stack Overflow why is heapify O(n)"*  
  Mathematical proof that building a heap bottom-up is linear, not O(n log n).

---

## C++ Language & Standard Library

### Books

| Title | Authors | Notes |
|---|---|---|
| *A Tour of C++*, 3rd ed. | Stroustrup | Best fast overview of modern C++ (C++20) by the language creator; ~250 pages |
| *Effective Modern C++* | Meyers | 42 practical items on C++11/14 features; move semantics, lambdas, `auto`, smart pointers |
| *C++ Templates: The Complete Guide*, 2nd ed. | Vandevoorde, Josuttis, Gregor | Reference if you want to go deeper on the template mechanics used in `Algorithm<In,Out>` |

### Online References

- **cppreference.com** — https://en.cppreference.com  
  The authoritative online reference for the C++ standard library. Use it to
  check container guarantees, algorithm complexity, and iterator requirements.

- **C++ Core Guidelines** — https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines  
  Authored by Stroustrup & Sutter. Canonical rules for writing safe, modern C++.

- **Compiler Explorer (Godbolt)** — https://godbolt.org  
  Paste any snippet and see the generated assembly for GCC, Clang, or MSVC at
  any optimisation level. Invaluable for understanding what the compiler does to
  your sort inner loops.

---

## Benchmarking & Performance

- **Catch2 Benchmark Docs** — https://github.com/catchorg/Catch2/blob/devel/docs/benchmarks.md  
  Official documentation for the `BENCHMARK` macro, warm-up, sample count,
  `--benchmark-*` CLI flags, and interpreting output.

- **Abseil Performance Tips** — https://abseil.io/fast  
  Short, practical notes on cache efficiency, branch prediction, memory layout,
  and why micro-benchmarks can mislead.

- **"What Every Programmer Should Know About Memory"** — Ulrich Drepper  
  https://people.freebsd.org/~lstewart/articles/cpumemory.pdf  
  Long but essential: cache lines, TLB, prefetching, NUMA. Explains why
  MergeSort's auxiliary array hurts cache more than QuickSort's in-place swaps.

- **perf** (Linux) / **VTune** (Windows/Linux)  
  Hardware performance counters — cache miss rates, branch mispredictions, IPC.
  Profile with these before micro-optimising.

---

## Testing with Catch2

- **Catch2 Documentation** — https://github.com/catchorg/Catch2/tree/devel/docs  
  Full reference: `TEST_CASE`, `SECTION`, `TEMPLATE_TEST_CASE`, matchers,
  generators, reporters, and the complete CLI flag list.

- **Catch2 Tutorial** — https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md  
  Start here if you are new to the framework.

---

## Build System (CMake)

- **CMake Tutorial** — https://cmake.org/cmake/help/latest/guide/tutorial/index.html  
  Official step-by-step guide from basic targets to packaging.

- **More Modern CMake** — Slides by Henry Scoffin  
  Search: *"More Modern CMake Henry Scoffin"*  
  Covers target-based CMake (the style used in this project) vs the old
  directory-based approach.

- **CMake by Example** — https://github.com/pr0g/cmake-examples  
  Short, focused examples for common patterns: FetchContent, testing, packaging.

---

## Logging (spdlog)

- **spdlog GitHub & Docs** — https://github.com/gabime/spdlog  
  API reference, sink types (console, file, rotating), async logging, and custom
  formatters. The `DSALog` singleton in this project wraps a multi-sink logger
  built from the patterns shown here.

---

## Practice Problem Sets

| Platform | Strength | URL |
|---|---|---|
| LeetCode | Huge problem bank, company tags | https://leetcode.com |
| Codeforces | Contest archive, strong community | https://codeforces.com |
| AtCoder | High-quality problems, beginner-friendly contests | https://atcoder.jp |
| CSES Problem Set | Curated 300-problem curriculum | https://cses.fi/problemset |
| Kattis | ACM-style judge, wide variety | https://open.kattis.com |

---

*See also: \ref framework_guide "Framework Guide" | \ref testing_guide "Testing & Benchmarking Guide" | \ref mainpage "Module overview"*
