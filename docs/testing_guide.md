# Testing & Benchmarking Guide {#testing_guide}

This project uses **Catch2 v3** for both correctness tests and micro-benchmarks.
All test files live in `test/` and are compiled into a single `tests` executable.

---

## Building

Tests are built automatically when `BUILD_TESTING=ON` (the default for top-level builds).

```bash
# Configure (Release for meaningful benchmark numbers)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build everything including the test binary
cmake --build build

# Or build only the test target
cmake --build build --target tests
```

The test binary is written to `build/test/tests` (or `build\test\tests.exe` on Windows).

---

## Running Tests

### Run all correctness tests (skip benchmarks)

Benchmarks are tagged `[benchmark]` and also carry `[!benchmark]`, which makes
Catch2 skip them by default. A plain run executes only correctness tests:

```bash
./build/test/tests
```

### Run tests for a specific module

Use a tag filter (all tags are lowercase module names):

```bash
./build/test/tests "[sorting]"
./build/test/tests "[graph]"
./build/test/tests "[dp]"
./build/test/tests "[heap]"
./build/test/tests "[trie]"
./build/test/tests "[tree]"
./build/test/tests "[searching]"
./build/test/tests "[linked_list]"
./build/test/tests "[segment_tree]"
./build/test/tests "[monotonic_queue]"
./build/test/tests "[hash_table]"
./build/test/tests "[bit_manip]"
./build/test/tests "[string_algo]"
```

### Run only correctness tests (explicitly)

```bash
./build/test/tests "[correctness]"
```

### Run a single named test case

```bash
./build/test/tests "Sorting - correctness"
./build/test/tests "Sorting - output is permutation"
```

### List all available test cases

```bash
./build/test/tests --list-tests
```

---

## Running Benchmarks

Benchmarks are opt-in. You must pass `--benchmark-samples` (or any benchmark flag)
to activate the `[!benchmark]` tests, **and** include them via a tag filter.

### Run all benchmarks

```bash
./build/test/tests "[benchmark]" --benchmark-samples 100
```

### Run benchmarks for a single module

```bash
./build/test/tests "[sorting][benchmark]"   --benchmark-samples 100
./build/test/tests "[graph][benchmark]"     --benchmark-samples 100
./build/test/tests "[heap][benchmark]"      --benchmark-samples 100
./build/test/tests "[searching][benchmark]" --benchmark-samples 100
```

### Run benchmarks only (skip all correctness tests)

```bash
./build/test/tests "[benchmark]" --benchmark-samples 200
```

---

## Useful Catch2 CLI Arguments

| Argument | What it does |
|---|---|
| `--benchmark-samples <N>` | Number of measurement samples per benchmark (default: 100). Higher = more stable results. |
| `--benchmark-warmup-time <ms>` | Milliseconds spent on warm-up iterations before measuring (default: 100). |
| `--benchmark-no-analysis` | Print raw sample times instead of statistical summaries. |
| `--reporter console` | Default human-readable output. |
| `--reporter xml` | Machine-readable XML — useful for CI. |
| `--reporter json` | JSON output. |
| `--reporter junit` | JUnit XML format for CI dashboards. |
| `-o <file>` / `--out <file>` | Write output to a file instead of stdout. |
| `--success` / `-s` | Also print passing assertions (very verbose). |
| `--abort` / `-a` | Abort on first test failure. |
| `--durations yes` | Print elapsed time for every test case. |
| `--list-tests` / `-l` | List all test cases and exit. |
| `--list-tags` | List all tags and exit. |
| `--order rand` | Randomise test execution order (good for catching hidden dependencies). |
| `--rng-seed <N>` | Fix the random seed used by `--order rand`. |
| `--verbosity quiet` | Suppress most output; only print failures. |
| `--verbosity high` | Print every section and assertion. |

### Examples combining flags

```bash
# Stable benchmark run: 500 samples, 200 ms warm-up, save results to file
./build/test/tests "[sorting][benchmark]" \
    --benchmark-samples 500 \
    --benchmark-warmup-time 200 \
    --reporter console \
    -o sorting_bench.txt

# CI-friendly: all tests, JUnit output, abort on first failure
./build/test/tests --reporter junit -o results.xml --abort

# Diagnose a flaky test: randomised order, fixed seed
./build/test/tests "[sorting]" --order rand --rng-seed 12345
```

---

## How Tests Are Structured

### Correctness tests — `TEMPLATE_TEST_CASE`

Each module uses `TEMPLATE_TEST_CASE` to run the same test body against every
algorithm implementation in that namespace. For example, in `test_sorting.cpp`:

```cpp
TEMPLATE_TEST_CASE("Sorting - correctness",
  "[sorting][correctness]",
  Sorting::QuickSort,
  Sorting::MergeSort,
  Sorting::HeapSort,
  Sorting::InsertionSort,
  Sorting::StdSort)
{
    TestType algo;          // instantiated once per type in the list

    SECTION("empty input") { ... }
    SECTION("random 1000 elements") { ... }
}
```

Every `SECTION` inside a `TEST_CASE` runs independently with a fresh fixture,
so a failing section does not prevent later sections from running.

### Benchmark tests — `BENCHMARK`

Benchmarks live inside a `TEST_CASE` tagged `[benchmark][!benchmark]`.
The `[!benchmark]` tag means Catch2 skips the case unless a benchmark flag
is explicitly provided on the command line.

```cpp
TEST_CASE("Sorting benchmarks", "[sorting][benchmark][!benchmark]")
{
    auto r1k = random_vec(1000);

    BENCHMARK("QuickSort random-1k")  { return Sorting::QuickSort{}.run(r1k); };
    BENCHMARK("MergeSort random-1k")  { return Sorting::MergeSort{}.run(r1k); };
    BENCHMARK("std::sort  random-1k") { return Sorting::StdSort{}.run(r1k);   };
}
```

Catch2 runs each `BENCHMARK` lambda many times (controlled by `--benchmark-samples`),
discards outliers, and reports min / mean / median / standard deviation automatically.

### Custom `TestSuite` (manual runner)

The framework also provides `TestSuite<In,Out>` and `Benchmarker` for use
**outside** Catch2 — for example, in `main.cpp` or quick ad-hoc scripts.
These use `std::chrono::high_resolution_clock` directly, run a configurable
number of repetitions, and log results through DSALog.

```cpp
// Example: run QuickSort and MergeSort against each other manually
TestSuite<Sorting::Vec, Sorting::Vec> suite("Sorting", /*reps=*/200);
suite.add_algo(std::make_shared<Sorting::QuickSort>())
     .add_algo(std::make_shared<Sorting::MergeSort>())
     .add_case("random-1k", random_vec(1000), expected)
     .run();   // prints comparison table via DSALog
```

Results are also written to `dsa_results.log` in the working directory.

---

## Adding a New Test File

1. Create `test/test_mymodule.cpp` — include `<catch2/catch_all.hpp>` and your header.
2. Add the file to the `tests` executable in `test/CMakeLists.txt`:
   ```cmake
   add_executable(tests
       ...
       test_mymodule.cpp
   )
   ```
3. Rebuild with `cmake --build build --target tests`.

---

*See also: \ref mainpage "Module overview"*
