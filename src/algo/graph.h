#pragma once
#include "dsa_framework/framework.h"
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------
//  Graph namespace
//  Graph is an adjacency list: vector<vector<pair<int,int>>>
//    Each entry is {neighbour, weight}
// -----------------------------------------------------------------------------
namespace Graph {

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
  int run(const BFSInput &in) override
  {
    int n = (int)in.graph.size();
    std::vector<int> dist(n, -1);
    std::queue<int> q;
    dist[in.source] = 0;
    q.push(in.source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      if (u == in.target) return dist[u];
      for (auto [v, _] : in.graph[u]) {
        if (dist[v] == -1) {
          dist[v] = dist[u] + 1;
          q.push(v);
        }
      }
    }
    return -1;// unreachable
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
  std::vector<int> run(const DijkstraInput &in) override
  {
    int n = (int)in.graph.size();
    const int INF = std::numeric_limits<int>::max();
    std::vector<int> dist(n, INF);
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
    dist[in.source] = 0;
    pq.push({ 0, in.source });
    while (!pq.empty()) {
      auto [d, u] = pq.top();
      pq.pop();
      if (d > dist[u]) continue;
      for (auto [v, w] : in.graph[u]) {
        if (dist[u] + w < dist[v]) {
          dist[v] = dist[u] + w;
          pq.push({ dist[v], v });
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
  std::vector<std::tuple<int, int, int>> edges;// {u, v, weight}
};

class BellmanFord : public Algorithm<EdgeListInput, std::vector<int>>
{
public:
  std::vector<int> run(const EdgeListInput &in) override
  {
    const int INF = std::numeric_limits<int>::max() / 2;
    std::vector<int> dist(in.num_nodes, INF);
    dist[in.source] = 0;
    for (int iter = 0; iter < in.num_nodes - 1; ++iter)
      for (auto [u, v, w] : in.edges)
        if (dist[u] != INF && dist[u] + w < dist[v]) dist[v] = dist[u] + w;
    return dist;
  }
  std::string name() const override { return "BellmanFord"; }
  std::string description() const override { return "O(VE), handles negative weights"; }
};

}// namespace Graph