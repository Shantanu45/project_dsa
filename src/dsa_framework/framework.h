#pragma once

// -- spdlog --------------------------------------------------------------------
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

// -- std -----------------------------------------------------------------------
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
//  DSALog  - singleton logger: coloured console + dsa_results.log
// =============================================================================
namespace DSALog
{
inline std::shared_ptr<spdlog::logger> get()
{
	static auto logger = []()
	{
		auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console->set_level(spdlog::level::debug);

		auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>("dsa_results.log", true);
		file->set_level(spdlog::level::trace);

		auto log = std::make_shared<spdlog::logger>("dsa", spdlog::sinks_init_list{console, file});
		log->set_level(spdlog::level::trace);
		log->set_pattern("[%T.%e] [%^%-5l%$] %v");
		spdlog::register_logger(log);
		return log;
	}();
	return logger;
}

template <typename... Args>
void info(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
	get()->info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
	get()->debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
	get()->warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void error(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
	get()->error(fmt, std::forward<Args>(args)...);
}
}  // namespace DSALog

// =============================================================================
//  Algorithm<In, Out>
// =============================================================================
template <typename In, typename Out>
class Algorithm
{
   public:
	virtual ~Algorithm() = default;
	virtual Out run(const In& input) = 0;
	virtual std::string name() const = 0;
	virtual std::string description() const { return ""; }
	virtual std::string complexity() const { return ""; }
	virtual bool validate(const Out& got, const Out& expected) const { return got == expected; }
};

// =============================================================================
//  TestCase<In, Out>
// =============================================================================
template <typename In, typename Out>
struct TestCase
{
	std::string label;
	In input;
	Out expected;
	TestCase(std::string l, In i, Out e) : label(std::move(l)), input(std::move(i)), expected(std::move(e)) {}
};

// =============================================================================
//  BenchmarkResult
// =============================================================================
struct BenchmarkResult
{
	std::string algo_name, test_label;
	double min_ns = 0, max_ns = 0, avg_ns = 0, median_ns = 0, stddev_ns = 0;
	int reps = 0;
	bool passed = false;

	double avg_us() const { return avg_ns / 1e3; }
	double avg_ms() const { return avg_ns / 1e6; }
	double median_us() const { return median_ns / 1e3; }
	double stddev_us() const { return stddev_ns / 1e3; }
};

// =============================================================================
//  Benchmarker  - manual ns timing used outside Catch2 contexts
// =============================================================================
class Benchmarker
{
   public:
	template <typename Fn>
	static std::vector<double> measure(Fn&& fn, int reps = 200)
	{
		std::vector<double> times;
		times.reserve(reps);
		// warm-up
		for (int i = 0; i < std::min(reps / 10, 5); ++i)
			fn();
		for (int i = 0; i < reps; ++i)
		{
			auto t0 = std::chrono::high_resolution_clock::now();
			fn();
			auto t1 = std::chrono::high_resolution_clock::now();
			times.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
		}
		return times;
	}

	static BenchmarkResult stats(const std::vector<double>& times, const std::string& algo = "", const std::string& label = "", bool passed = true)
	{
		BenchmarkResult r;
		r.algo_name = algo;
		r.test_label = label;
		r.reps = (int)times.size();
		r.passed = passed;
		auto s = times;
		std::sort(s.begin(), s.end());
		r.min_ns = s.front();
		r.max_ns = s.back();
		r.avg_ns = std::accumulate(s.begin(), s.end(), 0.0) / s.size();
		r.median_ns = s[s.size() / 2];
		double var = 0;
		for (double t : s)
			var += (t - r.avg_ns) * (t - r.avg_ns);
		r.stddev_ns = std::sqrt(var / s.size());
		return r;
	}
};

// =============================================================================
//  TestSuite<In,Out>  - manual runner (used in main.cpp; Catch2 uses its own)
// =============================================================================
template <typename In, typename Out>
class TestSuite
{
	using AlgoPtr = std::shared_ptr<Algorithm<In, Out>>;
	using Case = TestCase<In, Out>;

	std::string name_;
	std::vector<AlgoPtr> algos_;
	std::vector<Case> cases_;
	int reps_;

   public:
	explicit TestSuite(std::string name, int reps = 200) : name_(std::move(name)), reps_(reps) {}

	TestSuite& add_algo(AlgoPtr a)
	{
		algos_.push_back(std::move(a));
		return *this;
	}
	TestSuite& add_case(std::string label, In in, Out expected)
	{
		cases_.emplace_back(std::move(label), std::move(in), std::move(expected));
		return *this;
	}

	std::vector<BenchmarkResult> run() const
	{
		std::vector<BenchmarkResult> all;
		DSALog::info("");
		DSALog::info("=== {} ===", name_);

		for (auto& algo : algos_)
		{
			if (!algo->complexity().empty())
				DSALog::debug("  [{}]  {}", algo->name(), algo->complexity());
			for (auto& tc : cases_)
			{
				Out got = algo->run(tc.input);
				bool pass = algo->validate(got, tc.expected);
				auto times = Benchmarker::measure([&] { algo->run(tc.input); }, reps_);
				auto res = Benchmarker::stats(times, algo->name(), tc.label, pass);
				all.push_back(res);

				if (pass)
					DSALog::info(
						"  PASS  {:22s}  {:22s}  avg {:7.2f} us  "
						"med {:7.2f} us  sd {:6.2f} us",
						algo->name(), tc.label, res.avg_us(), res.median_us(), res.stddev_us());
				else
					DSALog::error("  FAIL  {:22s}  {:22s}", algo->name(), tc.label);
			}
		}

		if (algos_.size() > 1)
			_comparison(all);
		return all;
	}

   private:
	void _comparison(const std::vector<BenchmarkResult>& results) const
	{
		std::vector<std::string> tests, algos;
		for (auto& r : results)
		{
			if (!std::count(tests.begin(), tests.end(), r.test_label))
				tests.push_back(r.test_label);
			if (!std::count(algos.begin(), algos.end(), r.algo_name))
				algos.push_back(r.algo_name);
		}
		const int W = 14;
		std::ostringstream hdr;
		hdr << std::left << std::setw(22) << "Test";
		for (auto& a : algos)
			hdr << std::right << std::setw(W) << a.substr(0, W - 1);
		DSALog::info("");
		DSALog::info("Comparison (avg us):");
		DSALog::info("{}", hdr.str());
		DSALog::info("{}", std::string(22 + W * (int)algos.size(), '-'));
		for (auto& t : tests)
		{
			std::ostringstream row;
			row << std::left << std::setw(22) << t.substr(0, 21);
			for (auto& a : algos)
			{
				auto it = std::find_if(results.begin(), results.end(), [&](const BenchmarkResult& r) { return r.algo_name == a && r.test_label == t; });
				if (it != results.end())
					row << std::right << std::setw(W - 2) << std::fixed << std::setprecision(2) << it->avg_us() << "us";
				else
					row << std::right << std::setw(W) << "--";
			}
			DSALog::info("{}", row.str());
		}
	}
};

// =============================================================================
//  AlgoRegistry
// =============================================================================
template <typename In, typename Out>
class AlgoRegistry
{
	using AlgoPtr = std::shared_ptr<Algorithm<In, Out>>;
	using FactoryFn = std::function<AlgoPtr()>;
	std::map<std::string, FactoryFn> reg_;

   public:
	void Register(const std::string& key, FactoryFn f) { reg_[key] = std::move(f); }
	AlgoPtr get(const std::string& key) const
	{
		auto it = reg_.find(key);
		return it == reg_.end() ? nullptr : it->second();
	}
	std::vector<std::string> keys() const
	{
		std::vector<std::string> ks;
		for (auto& [k, _] : reg_)
			ks.push_back(k);
		return ks;
	}
};