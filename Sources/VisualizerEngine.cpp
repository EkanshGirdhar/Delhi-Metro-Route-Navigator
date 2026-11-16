#include "VisualizerEngine.h"
#include <algorithm>
#include <climits>

using namespace std;

std::vector<VisualizerStep> VisualizerEngine::generateDijkstraSteps(const MetroGraph& graph, int src, int dst) {
    std::vector<VisualizerStep> steps;
    auto adj = graph.buildAdjacencyList(false);
    if (adj.find(src) == adj.end()) return steps;

    std::unordered_map<int, int> dist;
    std::unordered_map<int, int> prevNode;
    std::vector<int> visitedList;
    std::unordered_set<int> visitedSet;

    for (const auto& pair : adj) {
        dist[pair.first] = INT_MAX;
    }
    dist[src] = 0;

    std::vector<std::pair<int, int>> pq = {{0, src}};

    auto formatQueue = [&]() {
        std::vector<std::string> qStr;
        auto sorted = pq;
        std::sort(sorted.begin(), sorted.end());
        for (const auto& item : sorted) {
            const Station* s = graph.getStation(item.second);
            qStr.push_back((s ? s->name : std::to_string(item.second)) + " (" + std::to_string(item.first) + ")");
        }
        return qStr;
    };

    steps.push_back({-1, {}, "Start at source station (dist 0)", dist, prevNode, visitedList, formatQueue()});

    while (!pq.empty()) {
        std::sort(pq.begin(), pq.end());
        int currentDist = pq.front().first;
        int u = pq.front().second;
        pq.erase(pq.begin());

        if (visitedSet.count(u)) continue;
        visitedSet.insert(u);
        visitedList.push_back(u);

        std::vector<int> usedEdges;
        std::vector<std::string> updates;

        if (u != dst) {
            for (const auto& edge : adj[u]) {
                if (visitedSet.count(edge.target)) continue;
                usedEdges.push_back(edge.edgeId);

                int newDist = dist[u] + edge.weight;
                if (newDist < dist[edge.target]) {
                    dist[edge.target] = newDist;
                    prevNode[edge.target] = u;
                    pq.push_back({newDist, edge.target});

                    const Station* targetStation = graph.getStation(edge.target);
                    std::string targetName = targetStation ? targetStation->name : std::to_string(edge.target);
                    updates.push_back(targetName + "=" + std::to_string(newDist));
                }
            }
        }

        const Station* currentStation = graph.getStation(u);
        std::string uName = currentStation ? currentStation->name : std::to_string(u);
        std::string note = "At " + uName + " (dist " + std::to_string(currentDist) + ").";

        if (u == dst) {
            note += " Reached destination " + uName + "!";
        } else if (!updates.empty()) {
            note += " Updated: ";
            for (size_t i = 0; i < updates.size(); ++i) {
                note += (i > 0 ? ", " : "") + updates[i];
            }
        } else {
            note += " No shorter neighbors.";
        }

        steps.push_back({u, usedEdges, note, dist, prevNode, visitedList, formatQueue()});
        if (u == dst) break;
    }

    return steps;
}

std::vector<VisualizerStep> VisualizerEngine::generateBFSSteps(const MetroGraph& graph, int src, int dst) {
    std::vector<VisualizerStep> steps;
    auto adj = graph.buildAdjacencyList(false);
    if (adj.find(src) == adj.end()) return steps;

    std::unordered_map<int, int> level;
    std::unordered_map<int, int> prevNode;
    std::vector<int> visitedList = {src};
    std::vector<int> queue = {src};

    for (const auto& pair : adj) {
        level[pair.first] = INT_MAX;
    }
    level[src] = 0;

    auto formatQueue = [&]() {
        std::vector<std::string> qStr;
        for (int sid : queue) {
            const Station* s = graph.getStation(sid);
            qStr.push_back(s ? s->name : std::to_string(sid));
        }
        return qStr;
    };

    steps.push_back({-1, {}, "Start at source station (0 stops)", level, prevNode, visitedList, formatQueue()});

    while (!queue.empty()) {
        int u = queue.front();
        queue.erase(queue.begin());

        std::vector<int> usedEdges;
        std::vector<std::string> found;

        if (u != dst) {
            for (const auto& edge : adj[u]) {
                if (level[edge.target] != INT_MAX) continue;
                level[edge.target] = level[u] + 1;
                prevNode[edge.target] = u;
                visitedList.push_back(edge.target);
                queue.push_back(edge.target);
                usedEdges.push_back(edge.edgeId);

                const Station* targetStation = graph.getStation(edge.target);
                found.push_back(targetStation ? targetStation->name : std::to_string(edge.target));
            }
        }

        const Station* currentStation = graph.getStation(u);
        std::string uName = currentStation ? currentStation->name : std::to_string(u);
        std::string note = "At " + uName + ".";

        if (u == dst) {
            note += " Reached destination " + uName + "!";
        } else if (!found.empty()) {
            note += " Next: ";
            for (size_t i = 0; i < found.size(); ++i) {
                note += (i > 0 ? ", " : "") + found[i];
            }
        } else {
            note += " No new neighbors.";
        }

        steps.push_back({u, usedEdges, note, level, prevNode, visitedList, formatQueue()});
        if (u == dst) break;
    }

    return steps;
}

void VisualizerEngine::dfsVisit(const MetroGraph& graph,
                                std::unordered_map<int, std::vector<AdjEdge>>& adj,
                                int u, int viaEdge, int dst,
                                std::unordered_map<int, int>& order,
                                std::unordered_map<int, int>& prevNode,
                                std::vector<int>& seenList,
                                std::vector<int>& stackList,
                                int& stepCount, bool& found,
                                std::vector<VisualizerStep>& steps) {
    seenList.push_back(u);
    order[u] = ++stepCount;
    stackList.push_back(u);

    std::vector<std::string> stackNames;
    for (int sid : stackList) {
        const Station* s = graph.getStation(sid);
        stackNames.push_back(s ? s->name : std::to_string(sid));
    }

    const Station* stationObj = graph.getStation(u);
    std::string stationName = stationObj ? stationObj->name : std::to_string(u);

    VisualizerStep step;
    step.currentStation = u;
    if (viaEdge != -1) step.activeEdges = {viaEdge};
    step.note = "Visit " + stationName;
    step.distances = order;
    step.previous = prevNode;
    step.visitedOrder = seenList;
    step.frontierQueue = stackNames;
    steps.push_back(step);

    if (u == dst) {
        found = true;
        VisualizerStep destStep = step;
        destStep.note = "Reached destination " + stationName + "!";
        steps.push_back(destStep);
        return;
    }

    for (const auto& edge : adj[u]) {
        if (order[edge.target] == INT_MAX) {
            prevNode[edge.target] = u;
            dfsVisit(graph, adj, edge.target, edge.edgeId, dst, order, prevNode, seenList, stackList, stepCount, found, steps);
            if (found) return;
        }
    }

    stackList.pop_back();

    stackNames.clear();
    for (int sid : stackList) {
        const Station* s = graph.getStation(sid);
        stackNames.push_back(s ? s->name : std::to_string(sid));
    }

    VisualizerStep backStep;
    backStep.currentStation = u;
    backStep.note = "Backtracking from " + stationName;
    backStep.distances = order;
    backStep.previous = prevNode;
    backStep.visitedOrder = seenList;
    backStep.frontierQueue = stackNames;
    steps.push_back(backStep);
}

std::vector<VisualizerStep> VisualizerEngine::generateDFSSteps(const MetroGraph& graph, int src, int dst) {
    std::vector<VisualizerStep> steps;
    auto adj = graph.buildAdjacencyList(false);
    if (adj.find(src) == adj.end()) return steps;

    std::unordered_map<int, int> order;
    std::unordered_map<int, int> prevNode;
    std::vector<int> seenList;
    std::vector<int> stackList;

    for (const auto& pair : adj) {
        order[pair.first] = INT_MAX;
    }

    int stepCount = 0;
    bool found = false;

    dfsVisit(graph, adj, src, -1, dst, order, prevNode, seenList, stackList, stepCount, found, steps);
    return steps;
}
