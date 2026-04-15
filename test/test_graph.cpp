#include "../src/dsa_framework/framework.h"
#include "../src/algo/graph.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <limits>

static const int INF = std::numeric_limits<int>::max() / 2;

// --- helper: build undirected weighted graph ----------------------------------
static Graph::AdjList make_graph(int n, const std::vector<std::tuple<int, int, int>> &edges)
{
  Graph::AdjList g(n);
  for (auto [u, v, w] : edges) {
    g[u].push_back({ v, w });
    g[v].push_back({ u, w });
  }
  return g;
}

// --- BFS ---------------------------------------------------------------------
TEST_CASE("BFS - hop distance", "[graph][bfs][correctness]")
{
  //  0 - 1 - 2 - 3 - 4    (chain)
  //  |_______________|     (0↔4 direct)
  auto g = make_graph(5, { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, 1 }, { 3, 4, 1 }, { 0, 4, 5 } });
  Graph::BFS bfs;

  REQUIRE(bfs.run({ g, 0, 0 }) == 0);// self
  REQUIRE(bfs.run({ g, 0, 1 }) == 1);// direct neighbor
  REQUIRE(bfs.run({ g, 0, 2 }) == 2);
  REQUIRE(bfs.run({ g, 0, 4 }) == 1);// direct edge 0-4 gives hop=1
  REQUIRE(bfs.run({ g, 1, 3 }) == 2);
}

TEST_CASE("BFS - unreachable node", "[graph][bfs][correctness]")
{
  // Two disconnected components: 0-1  and  2-3
  Graph::AdjList g(4);
  g[0].push_back({ 1, 1 });
  g[1].push_back({ 0, 1 });
  g[2].push_back({ 3, 1 });
  g[3].push_back({ 2, 1 });
  Graph::BFS bfs;
  REQUIRE(bfs.run({ g, 0, 3 }) == -1);
}

// --- Dijkstra -----------------------------------------------------------------
TEST_CASE("Dijkstra - basic SSSP", "[graph][dijkstra][correctness]")
{
  //  0 -1- 1 -2- 2
  //  |         /
  //  +---4----+
  auto g = make_graph(3, { { 0, 1, 1 }, { 1, 2, 2 }, { 0, 2, 4 } });
  Graph::Dijkstra dij;

  auto dist = dij.run({ g, 0 });
  REQUIRE(dist[0] == 0);
  REQUIRE(dist[1] == 1);
  REQUIRE(dist[2] == 3);// 0->1->2 cheaper than 0->2 direct
}

TEST_CASE("Dijkstra - 5-node chain with shortcut", "[graph][dijkstra][correctness]")
{
  auto g = make_graph(5, { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, 1 }, { 3, 4, 1 }, { 0, 4, 10 } });
  Graph::Dijkstra dij;
  auto d = dij.run({ g, 0 });
  REQUIRE(d == std::vector<int>{ 0, 1, 2, 3, 4 });
}

TEST_CASE("Dijkstra - single node graph", "[graph][dijkstra][correctness]")
{
  Graph::AdjList g(1);
  Graph::Dijkstra dij;
  auto d = dij.run({ g, 0 });
  REQUIRE(d[0] == 0);
}

// --- Bellman-Ford -------------------------------------------------------------
TEST_CASE("BellmanFord - handles negative weights", "[graph][bellman-ford][correctness]")
{
  // 0 -6- 1, 0 -7- 2, 1 -8- 2, 1 -(-4)- 3, 2 -9- 3, 2 -(-3)- 4, 3 -5- 4
  Graph::EdgeListInput in{
    5, 0, { { 0, 1, 6 }, { 0, 2, 7 }, { 1, 2, 8 }, { 1, 3, -4 }, { 2, 3, 9 }, { 2, 4, -3 }, { 3, 4, 5 } }
  };
  Graph::BellmanFord bf;
  auto d = bf.run(in);
  REQUIRE(d[0] == 0);
  REQUIRE(d[1] == 6);
  REQUIRE(d[2] == 7);
  REQUIRE(d[3] == 2);// 0->1 (6) + 1->3 (-4)
  REQUIRE(d[4] == 4);// 0->2 (7) + 2->4 (-3)
}

// --- benchmarks ---------------------------------------------------------------
TEST_CASE("Graph benchmarks", "[graph][benchmark][!benchmark]")
{
  // Build a denser graph: grid-like 100 nodes
  Graph::AdjList big(100);
  for (int i = 0; i < 99; ++i) {
    big[i].push_back({ i + 1, i + 1 });
    big[i + 1].push_back({ i, i + 1 });
    if (i + 10 < 100) {
      big[i].push_back({ i + 10, i + 10 });
      big[i + 10].push_back({ i, i + 10 });
    }
  }

  BENCHMARK("BFS       100-node grid") { return Graph::BFS{}.run({ big, 0, 99 }); };
  BENCHMARK("Dijkstra  100-node grid") { return Graph::Dijkstra{}.run({ big, 0 }); };
}