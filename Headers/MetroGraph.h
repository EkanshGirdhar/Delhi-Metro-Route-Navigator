#ifndef METROGRAPH_H
#define METROGRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

struct Station {
    int id = -1;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    std::set<std::string> lines;
};

struct Edge {
    int id = -1;
    int u = -1;
    int v = -1;
    int weight = 1;
    std::string line;
};

struct AdjEdge {
    int target;
    int weight;
    int edgeId;
    std::string line;
};

struct RouteResult {
    bool found = false;
    std::vector<int> stationPath;
    std::vector<int> edgePath;
    int totalTime = 0;
    int totalStops = 0;
    int interchanges = 0;
};

class MetroGraph {
public:
    MetroGraph();

    void clear();
    void loadDefaultNetwork();

    void addStation(int id, const std::string& name, double x, double y);
    void addEdge(int id, int u, int v, int weight, const std::string& line);
    bool removeStation(int id);
    bool removeEdge(int id);
    bool hasStation(int id) const;
    bool areLinked(int u, int v) const;

    void setStationFailed(int id, bool failed);
    void setEdgeFailed(int id, bool failed);
    bool isStationFailed(int id) const;
    bool isEdgeFailed(int id) const;
    void restoreAll();
    const std::unordered_set<int>& getFailedStations() const { return m_failedStations; }
    const std::unordered_set<int>& getFailedEdges() const { return m_failedEdges; }

    const std::unordered_map<int, Station>& getStations() const { return m_stations; }
    const std::unordered_map<int, Edge>& getEdges() const { return m_edges; }
    const Station* getStation(int id) const;
    const Edge* getEdge(int id) const;
    int getNextEdgeId() const { return m_nextEdgeId; }
    std::vector<std::pair<std::string, int>> getSortedStations() const;

    std::unordered_map<int, std::vector<AdjEdge>> buildAdjacencyList(bool ignoreFailures = false) const;

private:
    std::unordered_map<int, Station> m_stations;
    std::unordered_map<int, Edge> m_edges;
    std::unordered_set<int> m_failedStations;
    std::unordered_set<int> m_failedEdges;
    int m_nextEdgeId = 21;
};

#endif // METROGRAPH_H
