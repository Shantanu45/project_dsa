# Framework Guide {#framework_guide}

Everything in this project is built on four classes defined in
`src/dsa_framework/framework.h`: `Algorithm`, `TestCase`, `Benchmarker`, and
`TestSuite`. This page explains each one and walks through adding a brand-new
algorithm from scratch.

---

## Core Classes

### `Algorithm<In, Out>`

The base class every algorithm must inherit from.

```cpp
template <typename In, typename Out>
class Algorithm
{
public:
    virtual Out  run(const In& input)         = 0;   // implement this
    virtual std::string name()          const = 0;   // short display name
    virtual std::string description()   const { return ""; }  // optional
    virtual std::string complexity()    const { return ""; }  // optional
    virtual bool validate(const Out& got, const Out& expected) const
    {
        return got == expected;  // override for floating-point / approximate results
    }
};
```

`In` and `Out` can be any copyable type — a plain `int`, a `struct`, a
`std::vector`, a `std::pair`, etc.

---

### `TestCase<In, Out>`

A plain data holder: one labelled input/expected-output pair.

```cpp
template <typename In, typename Out>
struct TestCase
{
    std::string label;
    In          input;
    Out         expected;
};
```

Usually you don't construct these directly; `TestSuite::add_case` does it for you.

---

### `Benchmarker`

A static helper that times a callable with `std::chrono::high_resolution_clock`.

```cpp
// Run fn 200 times (after a short warm-up) and return raw nanosecond samples.
auto times = Benchmarker::measure([&] { algo.run(input); }, /*reps=*/200);

// Summarise into a BenchmarkResult (min / max / avg / median / stddev).
auto r = Benchmarker::stats(times, algo.name(), "my-label", /*passed=*/true);

// Convenience unit converters on BenchmarkResult:
r.avg_us()     // average in microseconds
r.median_us()  // median in microseconds
r.stddev_us()  // standard deviation in microseconds
```

The warm-up phase runs `min(reps/10, 5)` iterations before timing begins so
that branch predictors and caches reach steady state.

---

### `TestSuite<In, Out>`

A fluent builder that ties algorithms and test cases together, runs them, and
prints a comparison table via DSALog.

```cpp
TestSuite<In, Out> suite("Suite name", /*reps=*/200);
suite
    .add_algo(std::make_shared<MyAlgoA>())
    .add_algo(std::make_shared<MyAlgoB>())
    .add_case("label", input_value, expected_value)
    .add_case("label2", input_value2, expected_value2);

auto results = suite.run();
// results is std::vector<BenchmarkResult>
```

For each (algorithm × test-case) pair it:
1. Calls `algo.run(input)` and compares with `expected` via `algo.validate()`.
2. Runs `reps` timed repetitions and computes statistics.
3. Logs `PASS` / `FAIL` with timing numbers.
4. When more than one algorithm is registered, prints a side-by-side comparison
   table (average µs per test case).

Results are also written to `dsa_results.log` in the working directory.

---

### `DSALog`

A singleton logger that writes coloured output to the console **and** appends
to `dsa_results.log` simultaneously.

```cpp
DSALog::info("value = {}", 42);
DSALog::debug("detail = {}", x);
DSALog::warn("unexpected = {}", y);
DSALog::error("FAIL: {}", msg);
```

Use `info` for results, `debug` for verbose internals, `warn`/`error` for
problems. The format string follows [{fmt}](https://fmt.dev) syntax.

---

## Implementing a New Algorithm — Step by Step

The worked example below adds **Cycle Sort** to the `Sorting` namespace.

### Step 1 — Define the Input/Output types (if new)

`Sorting` already uses `using Vec = std::vector<int>;` so nothing new is needed.
For a brand-new domain, define a struct in the same header:

```cpp
struct MyInput
{
    std::vector<int> data;
    int parameter;
};
```

### Step 2 — Inherit from `Algorithm<In, Out>` and implement `run`

```cpp
// In src/algo/sorting.h, inside namespace Sorting:

/**
 * @brief In-place sort that minimises the number of writes to the array.
 *
 * @details
 * Cycle Sort decomposes the permutation into cycles.  Each element is moved
 * directly to its final sorted position, so every element is written at most
 * once.  This makes it the theoretical optimum for write-minimising sorts.
 *
 * @par Complexity
 *   Time  O(n²) — double nested scan to count smaller elements per cycle.
 *   Space O(1)  — in-place.
 *   Not stable.
 *
 * @par When to use
 *   Storage media where writes are expensive or wear-limited (e.g. flash/EEPROM).
 */
class CycleSort : public Algorithm<Vec, Vec>
{
public:
    Vec run(const Vec& input) override
    {
        Vec v = input;
        int n = (int)v.size();
        for (int cycle_start = 0; cycle_start < n - 1; ++cycle_start)
        {
            int item = v[cycle_start];
            int pos  = cycle_start;
            for (int i = cycle_start + 1; i < n; ++i)
                if (v[i] < item) ++pos;
            if (pos == cycle_start) continue;
            while (item == v[pos]) ++pos;
            std::swap(v[pos], item);
            while (pos != cycle_start)
            {
                pos = cycle_start;
                for (int i = cycle_start + 1; i < n; ++i)
                    if (v[i] < item) ++pos;
                while (item == v[pos]) ++pos;
                std::swap(v[pos], item);
            }
        }
        return v;
    }

    std::string name()        const override { return "CycleSort"; }
    std::string description() const override { return "O(n^2), minimises writes"; }
    std::string complexity()  const override { return "Time O(n²)  Space O(1)"; }
};
```

### Step 3 — Add it to the build

`src/CMakeLists.txt` already lists `algo/sorting.h` so no CMake change is needed
for a new class in an existing file.  If you create a **new** header, add it:

```cmake
add_executable(dsa main.cpp
    ...
    algo/my_new_module.h   # <-- add here
)
```

### Step 4 — Write a Catch2 test file

Create `test/test_sorting.cpp` (already exists — just add a new `TEST_CASE`) or
a new file for a new module.

```cpp
// In test/test_sorting.cpp — add CycleSort to the template list:
TEMPLATE_TEST_CASE("Sorting - correctness",
    "[sorting][correctness]",
    Sorting::QuickSort,
    Sorting::MergeSort,
    Sorting::CycleSort)     // <-- new entry
{
    TestType algo;
    SECTION("random 1000 elements")
    {
        auto input = random_vec(1000);
        auto expected = input;
        std::sort(expected.begin(), expected.end());
        REQUIRE(algo.run(input) == expected);
    }
}

// Add a benchmark block:
TEST_CASE("Sorting benchmarks", "[sorting][benchmark][!benchmark]")
{
    auto r1k = random_vec(1000);
    BENCHMARK("CycleSort random-1k") { return Sorting::CycleSort{}.run(r1k); };
}
```

### Step 5 — Use `TestSuite` for a quick manual run (optional)

You can drive the new algorithm from `src/main.cpp` without rebuilding the test
binary:

```cpp
#include "algo/sorting.h"
#include "dsa_framework/framework.h"

int main()
{
    using Vec = Sorting::Vec;

    auto expected = [](Vec v) { std::sort(v.begin(), v.end()); return v; };
    Vec  input    = {5, 3, 8, 1, 9, 2};

    TestSuite<Vec, Vec> suite("Sorting comparison", 300);
    suite.add_algo(std::make_shared<Sorting::QuickSort>())
         .add_algo(std::make_shared<Sorting::CycleSort>())
         .add_case("small-6",    input,             expected(input))
         .add_case("random-1k",  random_vec(1000),  {})   // expected filled in at runtime
         .run();
}
```

---

## Overriding `validate` for Non-Exact Outputs

The default `validate` uses `==`. Override it when exact equality is not
appropriate — for instance, a floating-point median:

```cpp
class MyMedian : public Algorithm<std::vector<double>, double>
{
public:
    double run(const std::vector<double>& in) override { ... }
    std::string name() const override { return "MyMedian"; }

    bool validate(const double& got, const double& expected) const override
    {
        return std::abs(got - expected) < 1e-9;
    }
};
```

---

## Checklist for a New Algorithm

- [ ] Inherit from `Algorithm<In, Out>` and implement `run`, `name`.
- [ ] Optionally implement `description`, `complexity`, `validate`.
- [ ] Add the class (or new header) to `src/CMakeLists.txt` if needed.
- [ ] Add to a `TEMPLATE_TEST_CASE` list in the relevant `test/test_*.cpp`.
- [ ] Add a `BENCHMARK` block tagged `[benchmark][!benchmark]`.
- [ ] Run `./build/test/tests "[yourmodule][correctness]"` to verify.
- [ ] Run `./build/test/tests "[yourmodule][benchmark]" --benchmark-samples 200` to profile.

---

*See also: \ref testing_guide "Testing & Benchmarking Guide" | \ref mainpage "Module overview"*
