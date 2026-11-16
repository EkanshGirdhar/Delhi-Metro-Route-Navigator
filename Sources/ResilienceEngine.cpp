#include "ResilienceEngine.h"
#include <algorithm>

using namespace std;

void ResilienceEngine::tarjanDFS(int u, int parentEdge, int& timer, const AdjMap& adj, IntMap& disc, IntMap& low, IntSet& artPoints, IntSet& bridges) {
    disc[u] = low[u] = ++timer;
    int children = 0;

    auto it = adj.find(u);
    if (it == adj.end()) return;

    for (const auto& edge : it->second) {
        if (edge.edgeId == parentEdge) continue;

        int v = edge.target;
        if (disc.find(v) != disc.end()) {
            low[u] = std::min(low[u], disc[v]);
        } else {
            children++;
            tarjanDFS(v, edge.edgeId, timer, adj, disc, low, artPoints, bridges);
            low[u] = std::min(low[u], low[v]);

            if (low[v] > disc[u]) {
                bridges.insert(edge.edgeId);
            }
            if (parentEdge != -1 && low[v] >= disc[u]) {
                artPoints.insert(u);
            }
        }
    }

    if (parentEdge == -1 && children > 1) {
        artPoints.insert(u);
    }
}

ResilienceAnalysis ResilienceEngine::analyze(const MetroGraph& graph) {
    ResilienceAnalysis analysis;
    auto adj = graph.buildAdjacencyList(false);

    std::vector<int> activeIds;
    for (const auto& pair : adj) {
        activeIds.push_back(pair.first);
    }
    analysis.totalActiveStations = static_cast<int>(activeIds.size());

    int groupCount = 0;
    std::unordered_map<int, int> componentSizes;

    for (int u : activeIds) {
        if (analysis.componentMap.find(u) != analysis.componentMap.end()) continue;

        int currentGroup = groupCount++;
        std::vector<int> q;
        int head = 0;
        q.push_back(u);
        analysis.componentMap[u] = currentGroup;
        int compSize = 0;

        while (head < static_cast<int>(q.size())) {
            int curr = q[head++];
            compSize++;

            for (const auto& edge : adj[curr]) {
                if (analysis.componentMap.find(edge.target) == analysis.componentMap.end()) {
                    analysis.componentMap[edge.target] = currentGroup;
                    q.push_back(edge.target);
                }
            }
        }
        componentSizes[currentGroup] = compSize;
    }
    analysis.connectedComponents = groupCount;

    int maxCompSize = 0;
    for (const auto& pair : componentSizes) {
        if (pair.second > maxCompSize) maxCompSize = pair.second;
    }
    analysis.cutOffStations = analysis.totalActiveStations - maxCompSize;

    std::unordered_map<int, int> disc;
    std::unordered_map<int, int> low;
    int timer = 0;

    for (int u : activeIds) {
        if (disc.find(u) == disc.end()) {
            tarjanDFS(u, -1, timer, adj, disc, low, analysis.articulationStations, analysis.bridgeEdges);
        }
    }

    return analysis;
}
