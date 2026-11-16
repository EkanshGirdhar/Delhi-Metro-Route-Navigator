#include "MetroGraph.h"
#include <algorithm>

using namespace std;

MetroGraph::MetroGraph() { loadDefaultNetwork(); }

void MetroGraph::clear() {
  m_stations.clear();
  m_edges.clear();
  m_failedStations.clear();
  m_failedEdges.clear();
  m_nextEdgeId = 1;
}

void MetroGraph::loadDefaultNetwork() {
  clear();
  struct SInit { int id; const char* name; double x, y; };
  const SInit stations[] = {
    {0, "Dwarka", 70, 330}, {1, "Janakpuri", 220, 330}, {2, "Rajiv Chowk", 500, 330},
    {3, "Mandi House", 720, 330}, {4, "Noida Sector", 930, 330}, {5, "Kashmere Gate", 220, 180},
    {6, "Civil Lines", 360, 100}, {7, "Azadpur", 540, 100}, {8, "Model Town", 720, 120},
    {9, "Anand Vihar", 860, 200}, {10, "Welcome", 220, 480}, {11, "Hauz Khas", 360, 560},
    {12, "Saket", 540, 560}, {13, "Kalkaji", 720, 510}, {14, "Botanical Garden", 880, 450},
    {15, "Central Secretariat", 500, 210}, {16, "Lajpat Nagar", 500, 450}
  };
  for (const auto& s : stations) addStation(s.id, s.name, s.x, s.y);

  struct EInit { int id, u, v, w; const char* line; };
  const EInit edges[] = {
    {0, 0, 1, 4, "Blue"}, {1, 1, 10, 4, "Orange"}, {2, 10, 11, 4, "Orange"},
    {3, 11, 12, 5, "Orange"}, {4, 12, 16, 4, "Green"}, {5, 16, 2, 5, "Green"},
    {6, 12, 13, 5, "Orange"}, {7, 13, 14, 4, "Orange"}, {8, 14, 4, 6, "Orange"},
    {9, 1, 2, 14, "Blue"}, {10, 2, 3, 12, "Blue"}, {11, 3, 4, 14, "Blue"},
    {12, 1, 5, 3, "Yellow"}, {13, 5, 6, 3, "Yellow"}, {14, 6, 7, 4, "Yellow"},
    {15, 7, 8, 4, "Yellow"}, {16, 8, 9, 3, "Yellow"}, {17, 9, 4, 4, "Yellow"},
    {18, 2, 15, 4, "Pink"}, {19, 15, 7, 5, "Pink"}, {20, 3, 13, 7, "Pink"}
  };
  for (const auto& e : edges) addEdge(e.id, e.u, e.v, e.w, e.line);
  m_nextEdgeId = 21;
}

void MetroGraph::addStation(int id, const std::string& name, double x, double y) {
  m_stations[id] = {id, name, x, y, {}};
}

void MetroGraph::addEdge(int id, int u, int v, int weight, const std::string& line) {
  m_edges[id] = {id, u, v, weight, line};
  if (m_stations.count(u)) m_stations[u].lines.insert(line);
  if (m_stations.count(v)) m_stations[v].lines.insert(line);
  if (id >= m_nextEdgeId) m_nextEdgeId = id + 1;
}

bool MetroGraph::removeStation(int id) {
  if (m_stations.erase(id) == 0) return false;
  m_failedStations.erase(id);
  std::vector<int> toRemove;
  for (const auto& p : m_edges) {
    if (p.second.u == id || p.second.v == id) toRemove.push_back(p.first);
  }
  for (int eid : toRemove) removeEdge(eid);
  return true;
}

bool MetroGraph::removeEdge(int id) {
  auto it = m_edges.find(id);
  if (it == m_edges.end()) return false;
  int u = it->second.u, v = it->second.v;
  m_edges.erase(it);
  m_failedEdges.erase(id);

  auto refreshLines = [this](int sid) {
    if (!m_stations.count(sid)) return;
    m_stations[sid].lines.clear();
    for (const auto& p : m_edges) {
      if (p.second.u == sid || p.second.v == sid) m_stations[sid].lines.insert(p.second.line);
    }
  };
  refreshLines(u);
  refreshLines(v);
  return true;
}

bool MetroGraph::hasStation(int id) const { return m_stations.count(id) > 0; }

bool MetroGraph::areLinked(int u, int v) const {
  for (const auto& p : m_edges) {
    if ((p.second.u == u && p.second.v == v) || (p.second.u == v && p.second.v == u)) return true;
  }
  return false;
}

void MetroGraph::setStationFailed(int id, bool failed) {
  if (failed) m_failedStations.insert(id); else m_failedStations.erase(id);
}

void MetroGraph::setEdgeFailed(int id, bool failed) {
  if (failed) m_failedEdges.insert(id); else m_failedEdges.erase(id);
}

bool MetroGraph::isStationFailed(int id) const { return m_failedStations.count(id) > 0; }
bool MetroGraph::isEdgeFailed(int id) const { return m_failedEdges.count(id) > 0; }

void MetroGraph::restoreAll() {
  m_failedStations.clear();
  m_failedEdges.clear();
}

const Station* MetroGraph::getStation(int id) const {
  auto it = m_stations.find(id);
  return it != m_stations.end() ? &(it->second) : nullptr;
}

const Edge* MetroGraph::getEdge(int id) const {
  auto it = m_edges.find(id);
  return it != m_edges.end() ? &(it->second) : nullptr;
}

std::unordered_map<int, std::vector<AdjEdge>> MetroGraph::buildAdjacencyList(bool ignoreFailures) const {
  std::unordered_map<int, std::vector<AdjEdge>> adj;
  for (const auto& p : m_stations) {
    if (ignoreFailures || !m_failedStations.count(p.first)) adj[p.first] = {};
  }
  for (const auto& p : m_edges) {
    const Edge& e = p.second;
    if (!ignoreFailures && (m_failedEdges.count(e.id) || m_failedStations.count(e.u) || m_failedStations.count(e.v))) continue;
    if (adj.count(e.u) && adj.count(e.v)) {
      adj[e.u].push_back({e.v, e.weight, e.id, e.line});
      adj[e.v].push_back({e.u, e.weight, e.id, e.line});
    }
  }
  return adj;
}

std::vector<std::pair<std::string, int>> MetroGraph::getSortedStations() const {
  std::vector<std::pair<std::string, int>> list;
  list.reserve(m_stations.size());
  for (const auto& p : m_stations) list.push_back({p.second.name, p.first});
  std::sort(list.begin(), list.end());
  return list;
}
