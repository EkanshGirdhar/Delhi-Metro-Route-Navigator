#ifndef VISUALIZERENGINE_H
#define VISUALIZERENGINE_H

#include "MetroGraph.h"
#include <string>
#include <vector>
#include <unordered_map>

struct VisualizerStep {
    int currentStation = -1;
    std::vector<int> activeEdges;
    std::string note;
    std::unordered_map<int, int> distances;
    std::unordered_map<int, int> previous;
    std::vector<int> visitedOrder;
    std::vector<std::string> frontierQueue;
};

class VisualizerEngine {
public:
    static std::vector<VisualizerStep> generateDijkstraSteps(const MetroGraph& graph, int src, int dst);
    static std::vector<VisualizerStep> generateBFSSteps(const MetroGraph& graph, int src, int dst);
    static std::vector<VisualizerStep> generateDFSSteps(const MetroGraph& graph, int src, int dst);

private:
    static void dfsVisit(const MetroGraph& graph,
                         std::unordered_map<int, std::vector<AdjEdge>>& adj,
                         int u, int viaEdge, int dst,
                         std::unordered_map<int, int>& order,
                         std::unordered_map<int, int>& prevNode,
                         std::vector<int>& seenList,
                         std::vector<int>& stackList,
                         int& stepCount, bool& found,
                         std::vector<VisualizerStep>& steps);
};

#endif // VISUALIZERENGINE_H
