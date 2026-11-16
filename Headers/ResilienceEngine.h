#ifndef RESILIENCEENGINE_H
#define RESILIENCEENGINE_H

#include "MetroGraph.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ResilienceAnalysis {
    int totalActiveStations = 0;
    int connectedComponents = 0;
    int cutOffStations = 0;
    std::unordered_set<int> articulationStations;
    std::unordered_set<int> bridgeEdges;
    std::unordered_map<int, int> componentMap;
};

class ResilienceEngine {
public:
    using AdjMap = std::unordered_map<int, std::vector<AdjEdge>>;
    using IntMap = std::unordered_map<int, int>;
    using IntSet = std::unordered_set<int>;

    static ResilienceAnalysis analyze(const MetroGraph& graph);

private:
    static void tarjanDFS(int u, int parentEdge, int& timer, const AdjMap& adj, IntMap& disc, IntMap& low, IntSet& artPoints, IntSet& bridges);
};

#endif // RESILIENCEENGINE_H
