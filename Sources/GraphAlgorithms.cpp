#include "GraphAlgorithms.h"
#include <queue>
#include <climits>
#include <algorithm>

using namespace std;

RouteResult GraphAlgorithms::findFastestRouteDijkstra(const MetroGraph& graph, int src, int dst, bool ignoreFailures) {
    auto adj = graph.buildAdjacencyList(ignoreFailures);
    if (adj.find(src) == adj.end() || adj.find(dst) == adj.end()) {
        return RouteResult();
    }

    int n = 0;
    for (const auto& pair : graph.getStations()) {
        n = max(n, pair.first);
    }
    n++;

    vector<int> dist(n, INT_MAX);
    vector<pair<int, int>> parent(n, {-1, -1});
    vector<bool> visited(n, false);

    dist[src] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if (visited[u]) continue;
        visited[u] = true;

        if (u == dst) break;

        for (const auto& edge : adj[u]) {
            int v = edge.target;
            int weight = edge.weight;

            if (!visited[v] && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = {u, edge.edgeId};
                pq.push({dist[v], v});
            }
        }
    }

    if (dist[dst] == INT_MAX) return RouteResult();
    return reconstructRoute(graph, parent, src, dst);
}

RouteResult GraphAlgorithms::findFewestStopsBFS(const MetroGraph& graph, int src, int dst, bool ignoreFailures) {
    auto adj = graph.buildAdjacencyList(ignoreFailures);
    if (adj.find(src) == adj.end() || adj.find(dst) == adj.end()) {
        return RouteResult();
    }

    int n = 0;
    for (const auto& pair : graph.getStations()) {
        n = max(n, pair.first);
    }
    n++;

    vector<bool> visited(n, false);
    vector<pair<int, int>> parent(n, {-1, -1});
    vector<int> queue;
    int head = 0;

    queue.push_back(src);
    visited[src] = true;

    while (head < static_cast<int>(queue.size())) {
        int u = queue[head++];

        if (u == dst) break;

        for (const auto& edge : adj[u]) {
            int v = edge.target;
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = {u, edge.edgeId};
                queue.push_back(v);
            }
        }
    }

    if (!visited[dst]) return RouteResult();
    return reconstructRoute(graph, parent, src, dst);
}

bool GraphAlgorithms::dfsHelper(int u, int target,
                                const unordered_map<int, vector<AdjEdge>>& adj,
                                vector<bool>& visited,
                                vector<pair<int, int>>& parent) {
    visited[u] = true;
    if (u == target) return true;

    auto it = adj.find(u);
    if (it != adj.end()) {
        for (const auto& edge : it->second) {
            int v = edge.target;
            if (!visited[v]) {
                parent[v] = {u, edge.edgeId};
                if (dfsHelper(v, target, adj, visited, parent)) {
                    return true;
                }
            }
        }
    }
    return false;
}

RouteResult GraphAlgorithms::findRouteDFS(const MetroGraph& graph, int src, int dst, bool ignoreFailures) {
    auto adj = graph.buildAdjacencyList(ignoreFailures);
    if (adj.find(src) == adj.end() || adj.find(dst) == adj.end()) {
        return RouteResult();
    }

    int n = 0;
    for (const auto& pair : graph.getStations()) {
        n = max(n, pair.first);
    }
    n++;

    vector<bool> visited(n, false);
    vector<pair<int, int>> parent(n, {-1, -1});

    if (!dfsHelper(src, dst, adj, visited, parent)) {
        return RouteResult();
    }

    return reconstructRoute(graph, parent, src, dst);
}

RouteResult GraphAlgorithms::reconstructRoute(const MetroGraph& graph,
                                             const vector<pair<int, int>>& parent,
                                             int src, int dst) {
    RouteResult result;

    if (src == dst) {
        result.found = true;
        result.stationPath = {src};
        return result;
    }

    int curr = dst;
    while (curr != src) {
        if (curr < 0 || curr >= static_cast<int>(parent.size()) || parent[curr].first == -1) {
            result.found = false;
            return result;
        }

        result.stationPath.push_back(curr);
        result.edgePath.push_back(parent[curr].second);
        curr = parent[curr].first;
    }
    result.stationPath.push_back(src);

    reverse(result.stationPath.begin(), result.stationPath.end());
    reverse(result.edgePath.begin(), result.edgePath.end());

    result.found = true;
    result.totalStops = static_cast<int>(result.edgePath.size());
    result.totalTime = 0;
    result.interchanges = 0;

    for (size_t i = 0; i < result.edgePath.size(); ++i) {
        const Edge* edge = graph.getEdge(result.edgePath[i]);
        if (edge) {
            result.totalTime += edge->weight;
            if (i > 0) {
                const Edge* prevEdge = graph.getEdge(result.edgePath[i - 1]);
                if (prevEdge && prevEdge->line != edge->line) {
                    result.interchanges++;
                }
            }
        }
    }

    return result;
}
