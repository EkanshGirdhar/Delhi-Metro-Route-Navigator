#ifndef GRAPHALGORITHMS_H
#define GRAPHALGORITHMS_H

#include "MetroGraph.h"
#include <vector>
#include <unordered_map>

class GraphAlgorithms {
public:
    static RouteResult findFastestRouteDijkstra(const MetroGraph& graph, int src, int dst, bool ignoreFailures = false);
    static RouteResult findFewestStopsBFS(const MetroGraph& graph, int src, int dst, bool ignoreFailures = false);
    static RouteResult findRouteDFS(const MetroGraph& graph, int src, int dst, bool ignoreFailures = false);

private:
    static bool dfsHelper(int u, int target,
                          const std::unordered_map<int, std::vector<AdjEdge>>& adj,
                          std::vector<bool>& visited,
                          std::vector<std::pair<int, int>>& parent);

    static RouteResult reconstructRoute(const MetroGraph& graph,
                                        const std::vector<std::pair<int, int>>& parent,
                                        int src, int dst);
};

#endif // GRAPHALGORITHMS_H
