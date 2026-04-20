# project_dsa

A personal C++ library of canonical data structures and algorithms. Every
implementation wraps the shared `Algorithm<In, Out>` interface so algorithms
can be swapped, tested, and benchmarked against each other with minimal
boilerplate.

## What's included

| Module | Highlights |
|---|---|
| Sorting | QuickSort, MergeSort, HeapSort, InsertionSort, CountingSort, std::sort |
| Searching | BinarySearch, LinearSearch, InterpolationSearch |
| Graph | BFS, DFS, Dijkstra, Bellman-Ford, Topological Sort, Kruskal MST, Union-Find |
| Dynamic Programming | Knapsack, LCS, Coin Change, Edit Distance, LIS, Matrix Chain |
| Trees | BST, AVL |
| Trie | Prefix trie with exact-match and prefix-count |
| Heap | MaxHeap, MinHeap, MedianHeap, IndexedMinHeap, TopK, Kth-Largest |
| Hash Table | Chaining, Open Addressing |
| Linked List | Singly-linked list, reverse, merge sorted |
| Segment Tree | Range-sum query + point update |
| Monotonic Queue | Sliding-window maximum |
| Bit Manipulation | Popcount, single-number (XOR) tricks |
| String Algorithms | Pattern matching primitives |

## Requirements

- CMake 3.29+
- C++23 compiler (MSVC 19.3+, GCC 13+, or Clang 16+)
- Dependencies are fetched automatically via CMake FetchContent (Catch2, spdlog, fmt, CLI11)

## Build

```bash
# Configure (Release recommended for benchmarks)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build everything
cmake --build build

# Build only tests
cmake --build build --target tests
```

## Run tests

```bash
# All correctness tests (benchmarks skipped by default)
./build/test/tests

# One module
./build/test/tests "[sorting]"
./build/test/tests "[graph]"

# All benchmarks
./build/test/tests "[benchmark]" --benchmark-samples 200
```

See the [Testing & Benchmarking Guide](docs/testing_guide.md) for the full
list of Catch2 CLI options.

## Docs

HTML documentation is generated with Doxygen:

```bash
cd docs
doxygen Doxyfile
# open docs/output/html/index.html
```

Pages included:
- **Module overview** — every class with complexity table
- **Framework guide** — how to use `Algorithm<In,Out>`, `TestSuite`, `Benchmarker`, and how to add a new algorithm
- **Testing & benchmarking guide** — build, run, and interpret Catch2 tests and benchmarks
- **Resources** — books, websites, and practice problem sets

## Project structure

```
src/
  algo/            # all algorithm headers (one per module)
  dsa_framework/   # Algorithm<>, TestSuite<>, Benchmarker, DSALog
  main.cpp
test/              # Catch2 correctness tests + BENCHMARK blocks
docs/              # Doxygen config and markdown pages
```

## Author

Shantanu Kumar — April 2026
