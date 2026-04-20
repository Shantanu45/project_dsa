# project\_dsa {#mainpage}

A personal C++ library of canonical data structures and algorithms, each implemented as a benchmarkable `Algorithm<In, Out>` unit that plugs into the shared DSALog framework.

---

## Modules

### Sorting — `namespace Sorting`

| Class | Strategy | Time (avg) | Space |
|---|---|---|---|
| `QuickSort` | Lomuto-partition divide-and-conquer | O(n log n) | O(log n) |
| `MergeSort` | Stable top-down merge | O(n log n) | O(n) |
| `HeapSort` | In-place max-heap | O(n log n) | O(1) |
| `BubbleSort` | Adjacent-swap passes | O(n²) | O(1) |
| `InsertionSort` | Shift-and-insert | O(n²) | O(1) |
| `CountingSort` | Integer frequency count | O(n + k) | O(k) |

---

### Searching — `namespace Searching`

| Class | Strategy | Time |
|---|---|---|
| `BinarySearch` | Halving on sorted array | O(log n) |
| `LinearSearch` | Sequential scan | O(n) |
| `InterpolationSearch` | Probe by value estimate | O(log log n) avg |

---

### Graph — `namespace Graph`

| Class | Purpose | Complexity |
|---|---|---|
| `BFS` | Shortest hop-count path | O(V + E) |
| `DFS` | Full graph traversal / reachability | O(V + E) |
| `Dijkstra` | Single-source shortest paths (non-negative weights) | O((V + E) log V) |
| `BellmanFord` | Single-source shortest paths (negative weights) | O(V·E) |
| `TopologicalSort` | Kahn's BFS in-degree algorithm | O(V + E) |
| `KruskalMST` | Minimum Spanning Tree via Union-Find | O(E log E) |
| `UnionFind` | Disjoint-set union with path compression + rank | O(α(n)) |

---

### Dynamic Programming — `namespace DP`

| Class | Problem |
|---|---|
| `Knapsack` | 0/1 Knapsack (max value under weight budget) |
| `LCS` | Longest Common Subsequence length |
| `CoinChange` | Minimum coins to reach a target sum |
| `EditDistance` | Levenshtein edit distance |
| `LISNaive` | Longest Increasing Subsequence — O(n²) DP |
| `LISBinarySearch` | Longest Increasing Subsequence — O(n log n) patience sort |
| `MatrixChainMult` | Matrix Chain Multiplication optimal parenthesisation |

---

### Trees — `namespace Tree`

| Class | Purpose |
|---|---|
| `BST` | Binary Search Tree: insert / search / inorder |
| `AVL` | Self-balancing AVL tree |
| `BSTSearch` / `AVLSearch` | Build-then-search benchmark drivers |
| `BSTSort` / `AVLSort` | Inorder traversal yields sorted output |
| `StdSetSearch` | `std::set` baseline for search benchmarks |

---

### Trie — `namespace Trie`

| Class | Purpose |
|---|---|
| `Trie` | 26-child prefix trie: insert / exact-match / prefix-count |
| `TrieSearch` | Build-then-search benchmark driver |
| `TriePrefixCount` | Count words matching a given prefix |
| `HashSetSearch` / `LinearPrefixCount` | `std::unordered_set` / linear-scan baselines |

---

### Heap — `namespace Heap`

| Class | Purpose |
|---|---|
| `MaxHeap` / `MinHeap` | Manual heap with push / pop / peek |
| `MedianHeap` | Dual-heap (max + min) for O(1) running median |
| `IndexedMinHeap` | Min-heap with O(log n) decrease-key |
| `TopKElements` / `TopKSort` | K largest elements via heap vs full sort |
| `KthLargestHeap` / `KthLargestNth` | Kth largest via heap vs `std::nth_element` |
| `MedianFinderHeap` / `MedianFinderSort` | Static array median: heap vs sort baseline |

---

### Hash Table — `namespace HashTable`

| Class | Strategy |
|---|---|
| `ChainingHashMap` | Separate chaining with linked lists |
| `OpenAddressHashMap` | Open addressing with linear probing |

---

### Linked List — `namespace LinkedList`

| Class | Purpose |
|---|---|
| `SinglyLinkedList` | Manual singly-linked list |
| `ListReverse` / `ArrayReverse` | In-place reversal: linked list vs `std::reverse` |
| `ListMergeSorted` / `ArrayMergeSorted` | Merge two sorted sequences |

---

### Segment Tree — `namespace SegTree`

| Class | Purpose |
|---|---|
| `SegmentTree` | Range-sum queries and point updates in O(log n) |

---

### Monotonic Queue — `namespace MonotonicQueue`

| Class | Purpose |
|---|---|
| `MonotonicDeque` | Sliding-window maximum in O(n) |

---

### Bit Manipulation — `namespace BitManip`

Utilities for popcount comparison and single-number isolation (XOR tricks).

---

### String Algorithms — `namespace StringAlgo`

Pattern matching and string processing primitives.

---

## Framework

All algorithm classes inherit from `Algorithm<Input, Output>` defined in `dsa_framework/framework.h`.
The framework provides:

- **DSALog** — singleton logger writing to console (coloured) and `dsa_results.log` simultaneously.
- Uniform `run(const Input&) -> Output` interface for every algorithm, making it trivial to swap implementations and benchmark them head-to-head.

---

---

- \subpage framework_guide
- \subpage testing_guide
- \subpage resources

---

*Author: Shantanu Kumar — April 2026*
