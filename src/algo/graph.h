/*****************************************************************//**
 * \file   graph.h
 * \brief  
 * 
 * \author Shantanu Kumar
 * \date   April 2026
 *********************************************************************/

#include <limits>
#include <numeric>
#include <queue>
#include <unordered_map>
#include <vector>
#include "dsa_framework/framework.h"

// -----------------------------------------------------------------------------
//  Graph namespace
//  Graph is an adjacency list: vector<vector<pair<int,int>>>
//    Each entry is {neighbour, weight}
// -----------------------------------------------------------------------------
namespace Graph
{

using AdjList = std::vector<std::vector<std::pair<int, int>>>;

// -- BFS Input/Output ----------------------------------------------------------
struct BFSInput
{
	AdjList graph;
	int source;
	int target;
};

// BFS: returns shortest path (hop count) between source and target
class BFS : public Algorithm<BFSInput, int>
{
   public:
	int run(const BFSInput& in) override
	{
		int n = (int)in.graph.size();
		std::vector<int> dist(n, -1);
		std::queue<int> q;
		dist[in.source] = 0;
		q.push(in.source);
		while (!q.empty())
		{
			int u = q.front();
			q.pop();
			if (u == in.target)
				return dist[u];
			for (auto [v, _] : in.graph[u])
			{
				if (dist[v] == -1)
				{
					dist[v] = dist[u] + 1;
					q.push(v);
				}
			}
		}
		return -1;	// unreachable
	}
	std::string name() const override { return "BFS"; }
	std::string description() const override { return "O(V+E), unweighted shortest path"; }
};

// -- Dijkstra Input/Output -----------------------------------------------------
struct DijkstraInput
{
	AdjList graph;
	int source;
};

// Dijkstra: returns shortest distances from source to all nodes
class Dijkstra : public Algorithm<DijkstraInput, std::vector<int>>
{
   public:
	std::vector<int> run(const DijkstraInput& in) override
	{
		int n = (int)in.graph.size();
		const int INF = std::numeric_limits<int>::max();
		std::vector<int> dist(n, INF);
		std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
		dist[in.source] = 0;
		pq.push({0, in.source});
		while (!pq.empty())
		{
			auto [d, u] = pq.top();
			pq.pop();
			if (d > dist[u])
				continue;
			for (auto [v, w] : in.graph[u])
			{
				if (dist[u] + w < dist[v])
				{
					dist[v] = dist[u] + w;
					pq.push({dist[v], v});
				}
			}
		}
		return dist;
	}
	std::string name() const override { return "Dijkstra"; }
	std::string description() const override { return "O((V+E) log V), non-negative weights"; }
};

// -- Bellman-Ford (handles negative weights) -----------------------------------
struct EdgeListInput
{
	int num_nodes;
	int source;
	std::vector<std::tuple<int, int, int>> edges;  // {u, v, weight}
};

class BellmanFord : public Algorithm<EdgeListInput, std::vector<int>>
{
   public:
	std::vector<int> run(const EdgeListInput& in) override
	{
		const int INF = std::numeric_limits<int>::max() / 2;
		std::vector<int> dist(in.num_nodes, INF);
		dist[in.source] = 0;
		for (int iter = 0; iter < in.num_nodes - 1; ++iter)
			for (auto [u, v, w] : in.edges)
				if (dist[u] != INF && dist[u] + w < dist[v])
					dist[v] = dist[u] + w;
		return dist;
	}
	std::string name() const override { return "BellmanFord"; }
	std::string description() const override { return "O(VE), handles negative weights"; }
};

// -- DFS -----------------------------------------------------------------------
/**
 * Depth-First Search explores as far as possible along each branch before
 * backtracking.  Implemented recursively here for clarity.
 *
 * Returns the DFS visitation order starting from 'source'.
 * Unreachable nodes are not included.
 *
 * Complexities  O(V + E)
 *
 * Key uses (beyond simple traversal):
 *   - Cycle detection
 *   - Topological sort (via finish times)
 *   - Strongly Connected Components (Kosaraju / Tarjan)
 *   - Solving mazes, flood-fill
 *
 * Compared to BFS: DFS uses O(h) stack space (h = max depth) vs BFS's O(w)
 * queue space (w = max breadth).  For deep narrow graphs DFS is cheaper;
 * for wide shallow graphs BFS is cheaper.
 */
struct DFSInput
{
	AdjList graph;
	int     source;
};

class DFS : public Algorithm<DFSInput, std::vector<int>>
{
	void _dfs(const AdjList& g, int u, std::vector<bool>& visited, std::vector<int>& order) const
	{
		visited[u] = true;
		order.push_back(u);
		for (auto [v, _] : g[u])
			if (!visited[v])
				_dfs(g, v, visited, order);
	}

   public:
	std::vector<int> run(const DFSInput& in) override
	{
		int n = (int)in.graph.size();
		std::vector<bool> visited(n, false);
		std::vector<int>  order;
		_dfs(in.graph, in.source, visited, order);
		return order;
	}
	std::string name()        const override { return "DFS"; }
	std::string description() const override { return "O(V+E) depth-first traversal"; }
};

// -- Topological Sort (Kahn's BFS / in-degree algorithm) ----------------------
/**
 * A topological ordering of a DAG is a linear ordering of vertices such that
 * for every directed edge u → v, u appears before v.
 *
 * Kahn's algorithm:
 *   1. Compute in-degree for every vertex.
 *   2. Enqueue all vertices with in-degree 0 (no prerequisites).
 *   3. While queue non-empty: pop u, emit u, decrement in-degree of u's
 *      neighbours.  If a neighbour's in-degree hits 0, enqueue it.
 *   4. If result size < V, the graph has a cycle (no valid ordering exists).
 *
 * Complexities  O(V + E)
 *
 * Classic uses: build systems (Makefile), course prerequisite scheduling,
 * package dependency resolution, spreadsheet cell evaluation order.
 */
struct TopoInput
{
	int                            num_nodes;
	std::vector<std::pair<int,int>> edges;  // directed: {u → v}
};

class TopologicalSort : public Algorithm<TopoInput, std::vector<int>>
{
   public:
	std::vector<int> run(const TopoInput& in) override
	{
		int n = in.num_nodes;
		std::vector<std::vector<int>> adj(n);
		std::vector<int> indegree(n, 0);
		for (auto [u, v] : in.edges) { adj[u].push_back(v); ++indegree[v]; }

		std::queue<int> q;
		for (int i = 0; i < n; ++i)
			if (indegree[i] == 0) q.push(i);

		std::vector<int> order;
		order.reserve(n);
		while (!q.empty())
		{
			int u = q.front(); q.pop();
			order.push_back(u);
			for (int v : adj[u])
				if (--indegree[v] == 0) q.push(v);
		}
		// order.size() < n  ⟹  cycle detected
		return order;
	}
	std::string name()        const override { return "TopologicalSort"; }
	std::string description() const override { return "O(V+E) Kahn's BFS-based topo sort"; }
};

// -- Union-Find (Disjoint Set Union) ------------------------------------------
/**
 * Union-Find maintains a partition of {0 … n-1} into disjoint sets.
 * Two optimisations bring the amortised cost per operation to O(α(n)) ≈ O(1):
 *
 *   Path compression (find): flatten the tree so every node points directly
 *     to the root after a find call.
 *
 *   Union by rank: always attach the shorter tree under the taller one,
 *     keeping tree height O(log n) in the worst case.
 *
 * Complexities
 *   find      O(α(n)) amortised  (α = inverse Ackermann, ≤ 4 in practice)
 *   unite     O(α(n)) amortised
 *   connected O(α(n)) amortised
 *
 * Classic uses: Kruskal's MST, cycle detection in undirected graphs,
 * connected components, dynamic connectivity.
 */
class UnionFind
{
	std::vector<int> parent_, rank_;
	int              components_;

   public:
	explicit UnionFind(int n) : parent_(n), rank_(n, 0), components_(n)
	{
		std::iota(parent_.begin(), parent_.end(), 0);
	}

	// Path-compressed find
	int find(int x)
	{
		if (parent_[x] != x) parent_[x] = find(parent_[x]);
		return parent_[x];
	}

	// Union by rank — returns false if x and y were already in the same set
	bool unite(int x, int y)
	{
		int px = find(x), py = find(y);
		if (px == py) return false;
		if (rank_[px] < rank_[py]) std::swap(px, py);
		parent_[py] = px;
		if (rank_[px] == rank_[py]) ++rank_[px];
		--components_;
		return true;
	}

	bool connected(int x, int y) { return find(x) == find(y); }
	int  components() const      { return components_; }
	int  size()       const      { return (int)parent_.size(); }
};

// -- Kruskal's MST: uses UnionFind to build Minimum Spanning Tree -------------
/**
 * Kruskal's algorithm builds an MST by greedily adding the cheapest edge that
 * does not create a cycle.  Union-Find detects cycles in O(α(n)) per edge.
 *
 * Steps:
 *   1. Sort all edges by weight  O(E log E)
 *   2. For each edge (u, v, w) in order:
 *        if u and v are in different components → add edge, unite them
 *   3. Stop after V-1 edges are added (spanning tree is complete)
 *
 * Complexities  O(E log E)  dominated by sorting
 *
 * Returns total weight of the MST.
 * (For disconnected graphs, returns the weight of the minimum spanning forest.)
 */
struct KruskalInput
{
	int                                    num_nodes;
	std::vector<std::tuple<int, int, int>> edges;  // {weight, u, v}
};

class KruskalMST : public Algorithm<KruskalInput, int>
{
   public:
	int run(const KruskalInput& in) override
	{
		auto edges = in.edges;
		std::sort(edges.begin(), edges.end());  // sort by weight (first element)
		UnionFind uf(in.num_nodes);
		int total = 0;
		for (auto [w, u, v] : edges)
			if (uf.unite(u, v)) total += w;
		return total;
	}
	std::string name()        const override { return "KruskalMST"; }
	std::string description() const override { return "O(E log E) MST via UnionFind"; }
	std::string complexity()  const override { return "O(E log E) sort + O(E α(V)) union-find"; }
};

}  // namespace Graph