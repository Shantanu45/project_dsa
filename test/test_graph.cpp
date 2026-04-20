#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <limits>
#include "../src/algo/graph.h"
#include "../src/dsa_framework/framework.h"

static const int INF = std::numeric_limits<int>::max() / 2;

// --- helper: build undirected weighted graph ----------------------------------
static Graph::AdjList make_graph(int n, const std::vector<std::tuple<int, int, int>>& edges)
{
	Graph::AdjList g(n);
	for (auto [u, v, w] : edges)
	{
		g[u].push_back({v, w});
		g[v].push_back({u, w});
	}
	return g;
}

// --- BFS ---------------------------------------------------------------------
TEST_CASE("BFS - hop distance", "[graph][bfs][correctness]")
{
	//  0 - 1 - 2 - 3 - 4    (chain)
	//  |_______________|     (0↔4 direct)
	auto g = make_graph(5, {{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}, {0, 4, 5}});
	Graph::BFS bfs;

	REQUIRE(bfs.run({g, 0, 0}) == 0);  // self
	REQUIRE(bfs.run({g, 0, 1}) == 1);  // direct neighbor
	REQUIRE(bfs.run({g, 0, 2}) == 2);
	REQUIRE(bfs.run({g, 0, 4}) == 1);  // direct edge 0-4 gives hop=1
	REQUIRE(bfs.run({g, 1, 3}) == 2);
}

TEST_CASE("BFS - unreachable node", "[graph][bfs][correctness]")
{
	// Two disconnected components: 0-1  and  2-3
	Graph::AdjList g(4);
	g[0].push_back({1, 1});
	g[1].push_back({0, 1});
	g[2].push_back({3, 1});
	g[3].push_back({2, 1});
	Graph::BFS bfs;
	REQUIRE(bfs.run({g, 0, 3}) == -1);
}

// --- Dijkstra -----------------------------------------------------------------
TEST_CASE("Dijkstra - basic SSSP", "[graph][dijkstra][correctness]")
{
	//  0 -1- 1 -2- 2
	//  |         /
	//  +---4----+
	auto g = make_graph(3, {{0, 1, 1}, {1, 2, 2}, {0, 2, 4}});
	Graph::Dijkstra dij;

	auto dist = dij.run({g, 0});
	REQUIRE(dist[0] == 0);
	REQUIRE(dist[1] == 1);
	REQUIRE(dist[2] == 3);	// 0->1->2 cheaper than 0->2 direct
}

TEST_CASE("Dijkstra - 5-node chain with shortcut", "[graph][dijkstra][correctness]")
{
	auto g = make_graph(5, {{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}, {0, 4, 10}});
	Graph::Dijkstra dij;
	auto d = dij.run({g, 0});
	REQUIRE(d == std::vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("Dijkstra - single node graph", "[graph][dijkstra][correctness]")
{
	Graph::AdjList g(1);
	Graph::Dijkstra dij;
	auto d = dij.run({g, 0});
	REQUIRE(d[0] == 0);
}

// --- Bellman-Ford -------------------------------------------------------------
TEST_CASE("BellmanFord - handles negative weights", "[graph][bellman-ford][correctness]")
{
	// 0 -6- 1, 0 -7- 2, 1 -8- 2, 1 -(-4)- 3, 2 -9- 3, 2 -(-3)- 4, 3 -5- 4
	Graph::EdgeListInput in{5, 0, {{0, 1, 6}, {0, 2, 7}, {1, 2, 8}, {1, 3, -4}, {2, 3, 9}, {2, 4, -3}, {3, 4, 5}}};
	Graph::BellmanFord bf;
	auto d = bf.run(in);
	REQUIRE(d[0] == 0);
	REQUIRE(d[1] == 6);
	REQUIRE(d[2] == 7);
	REQUIRE(d[3] == 2);	 // 0->1 (6) + 1->3 (-4)
	REQUIRE(d[4] == 4);	 // 0->2 (7) + 2->4 (-3)
}

// --- DFS ----------------------------------------------------------------------
TEST_CASE("DFS - visitation order and reachability", "[graph][dfs][correctness]")
{
	// Linear chain: 0-1-2-3-4
	auto g = make_graph(5, {{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}});
	Graph::DFS dfs;

	SECTION("visits all nodes in a connected graph")
	{
		auto order = dfs.run({g, 0});
		REQUIRE(order.size() == 5);
		std::vector<int> sorted_order = order;
		std::sort(sorted_order.begin(), sorted_order.end());
		REQUIRE(sorted_order == std::vector<int>{0, 1, 2, 3, 4});
	}
	SECTION("source is always first")
	{
		REQUIRE(dfs.run({g, 0}).front() == 0);
		REQUIRE(dfs.run({g, 2}).front() == 2);
	}
	SECTION("disconnected graph - only reachable nodes visited")
	{
		Graph::AdjList h(4);
		h[0].push_back({1, 1}); h[1].push_back({0, 1});
		h[2].push_back({3, 1}); h[3].push_back({2, 1});
		auto order = dfs.run({h, 0});
		REQUIRE(order.size() == 2);
		REQUIRE(std::find(order.begin(), order.end(), 0) != order.end());
		REQUIRE(std::find(order.begin(), order.end(), 1) != order.end());
	}
	SECTION("single node graph")
	{
		Graph::AdjList h(1);
		auto order = dfs.run({h, 0});
		REQUIRE(order == std::vector<int>{0});
	}
}

// --- Topological Sort ----------------------------------------------------------
TEST_CASE("TopologicalSort - valid orderings", "[graph][topo][correctness]")
{
	Graph::TopologicalSort topo;

	SECTION("simple chain 0->1->2->3")
	{
		auto order = topo.run({4, {{0,1},{1,2},{2,3}}});
		REQUIRE(order.size() == 4);
		// each node must appear before the one it points to
		auto pos = [&](int v){ return std::find(order.begin(), order.end(), v) - order.begin(); };
		REQUIRE(pos(0) < pos(1));
		REQUIRE(pos(1) < pos(2));
		REQUIRE(pos(2) < pos(3));
	}
	SECTION("no edges - any permutation is valid, all nodes present")
	{
		auto order = topo.run({4, {}});
		REQUIRE(order.size() == 4);
		std::vector<int> sorted = order;
		std::sort(sorted.begin(), sorted.end());
		REQUIRE(sorted == std::vector<int>{0,1,2,3});
	}
	SECTION("diamond DAG: 0->1, 0->2, 1->3, 2->3")
	{
		auto order = topo.run({4, {{0,1},{0,2},{1,3},{2,3}}});
		REQUIRE(order.size() == 4);
		auto pos = [&](int v){ return std::find(order.begin(), order.end(), v) - order.begin(); };
		REQUIRE(pos(0) < pos(1));
		REQUIRE(pos(0) < pos(2));
		REQUIRE(pos(1) < pos(3));
		REQUIRE(pos(2) < pos(3));
	}
	SECTION("cycle detected - result size < num_nodes")
	{
		// 0->1->2->0 is a cycle
		auto order = topo.run({3, {{0,1},{1,2},{2,0}}});
		REQUIRE(order.size() < 3);
	}
}

// --- UnionFind -----------------------------------------------------------------
TEST_CASE("UnionFind - basic operations", "[graph][unionfind][correctness]")
{
	SECTION("initially all nodes are separate components")
	{
		Graph::UnionFind uf(5);
		REQUIRE(uf.components() == 5);
		for (int i = 0; i < 5; ++i)
			for (int j = i + 1; j < 5; ++j)
				REQUIRE_FALSE(uf.connected(i, j));
	}
	SECTION("unite merges components")
	{
		Graph::UnionFind uf(4);
		uf.unite(0, 1);
		REQUIRE(uf.connected(0, 1));
		REQUIRE(uf.components() == 3);
		uf.unite(2, 3);
		REQUIRE(uf.connected(2, 3));
		REQUIRE(uf.components() == 2);
		uf.unite(1, 2);
		REQUIRE(uf.connected(0, 3));
		REQUIRE(uf.components() == 1);
	}
	SECTION("unite returns false for already-connected nodes")
	{
		Graph::UnionFind uf(3);
		REQUIRE(uf.unite(0, 1) == true);
		REQUIRE(uf.unite(0, 1) == false);
		REQUIRE(uf.unite(1, 0) == false);
	}
	SECTION("path compression preserves correctness under many operations")
	{
		Graph::UnionFind uf(10);
		for (int i = 0; i < 9; ++i) uf.unite(i, i + 1);
		REQUIRE(uf.components() == 1);
		for (int i = 0; i < 10; ++i)
			for (int j = i + 1; j < 10; ++j)
				REQUIRE(uf.connected(i, j));
	}
}

// --- KruskalMST ----------------------------------------------------------------
TEST_CASE("KruskalMST - minimum spanning tree weight", "[graph][kruskal][correctness]")
{
	Graph::KruskalMST kruskal;

	SECTION("simple triangle: pick the two cheaper edges")
	{
		// weights: 0-1=1, 1-2=2, 0-2=10  →  MST = 1+2 = 3
		REQUIRE(kruskal.run({3, {{1,0,1},{2,1,2},{10,0,2}}}) == 3);
	}
	SECTION("single node, no edges")
	{
		REQUIRE(kruskal.run({1, {}}) == 0);
	}
	SECTION("chain graph: MST is the whole chain")
	{
		// 0-1=1, 1-2=2, 2-3=3  →  MST = 1+2+3 = 6
		REQUIRE(kruskal.run({4, {{1,0,1},{2,1,2},{3,2,3}}}) == 6);
	}
	SECTION("classic 4-node example")
	{
		// 0-1=10, 0-2=6, 0-3=5, 1-3=15, 2-3=4
		// Kruskal picks: 2-3=4, 0-3=5, 0-1=10 (0-2=6 skipped: 0 already reaches 2 via 0-3-2)
		// MST = 4+5+10 = 19
		REQUIRE(kruskal.run({4, {{10,0,1},{6,0,2},{5,0,3},{15,1,3},{4,2,3}}}) == 19);
	}
	SECTION("all equal weights")
	{
		// complete graph K4, all weight 1 → MST picks any 3 edges = 3
		REQUIRE(kruskal.run({4, {{1,0,1},{1,0,2},{1,0,3},{1,1,2},{1,1,3},{1,2,3}}}) == 3);
	}
}

// --- benchmarks ---------------------------------------------------------------
TEST_CASE("Graph benchmarks", "[graph][benchmark][!benchmark]")
{
	// Build a denser graph: grid-like 100 nodes
	Graph::AdjList big(100);
	for (int i = 0; i < 99; ++i)
	{
		big[i].push_back({i + 1, i + 1});
		big[i + 1].push_back({i, i + 1});
		if (i + 10 < 100)
		{
			big[i].push_back({i + 10, i + 10});
			big[i + 10].push_back({i, i + 10});
		}
	}

	BENCHMARK("BFS       100-node grid")
	{
		return Graph::BFS{}.run({big, 0, 99});
	};
	BENCHMARK("Dijkstra  100-node grid")
	{
		return Graph::Dijkstra{}.run({big, 0});
	};
	BENCHMARK("DFS       100-node grid")
	{
		return Graph::DFS{}.run({big, 0});
	};
}