# Metro Route Navigation System — Technical Evaluation & Viva Defense Guide

This document is a comprehensive technical guide to the **Delhi Metro Route Navigation System**. It is structured to help you confidently explain the architecture, data structures, algorithms, and design choices during project evaluation, code review, and viva examinations.

---

## 1. System Overview & Architecture

The application is built using a clean **three-layer architecture**:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        1. Presentation Layer (Qt 6)                     │
│  MainWindow      : Central coordinator and sidebar navigation           │
│  MetroMapView    : Custom 2D vector canvas (QPainter, hit-testing)      │
│  NavigatorPage   : Route calculator, Dijkstra vs BFS comparison         │
│  VisualizerPage  : Animated algorithm visualizer with step controls     │
│  ResiliencePage  : Fault injection lab, single-point-of-failure table   │
│  ManagementPage  : In-memory station directory and track editor         │
│  AppTheme.h      : Dark color palette and custom Qt stylesheet (QSS)   │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │
┌────────────────────────────────────▼────────────────────────────────────┐
│                    2. Algorithm & Computation Layer                     │
│  GraphAlgorithms   : Dijkstra (Min-Heap), BFS, DFS, Route reconstruction│
│  VisualizerEngine  : Step snapshot recorder for algorithm animation     │
│  ResilienceEngine  : Tarjan's Bridge & Articulation Point detection     │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │
┌────────────────────────────────────▼────────────────────────────────────┐
│                      3. Data & Graph Model Layer                        │
│  MetroGraph        : In-memory network model, stations, edges,          │
│                      adjacency list, connectivity checks                │
└─────────────────────────────────────────────────────────────────────────┘
```

### Module Responsibilities:
1. **`MetroGraph`**: Encapsulates the graph topology. Manages vertices (`Station`), edges (`Edge`), and builds an adjacency list.
2. **`GraphAlgorithms`**: Pure functional algorithm implementations that take graph data and return computed route paths.
3. **`VisualizerEngine`**: Intercepts the search process to record an immutable sequence of snapshots (`VisualizerStep`) representing algorithm progress.
4. **`ResilienceEngine`**: Implements Tarjan's algorithm to compute network vulnerability metrics.
5. **UI Pages (`NavigatorPage`, `VisualizerPage`, etc.)**: Pure presentation components receiving user inputs, invoking algorithms, and rendering results.

---

## 2. Core Functional Modules

### 1. Metro Navigator
- Accepts an origin station and a destination station.
- Allows user selection between **Dijkstra's Algorithm** (fastest travel time) and **BFS** (fewest stops).
- Displays journey statistics (total time, number of stations, line transfers).
- Renders an interactive vertical step-by-step travel itinerary with line transfer notices and highlights the entire path on the 2D map.

### 2. Algorithm Visualizer
- Visualizes the inner workings of Dijkstra, BFS, and DFS directly on the metro map.
- Uses color-coded states for nodes:
  - **Unvisited** (Dark Gray)
  - **In Queue / Frontier** (Orange)
  - **Currently Visiting / Relaxing** (Cyan)
  - **Visited / Settled** (Green)
- Provides **Play / Pause**, **Next Step**, and **Reset** controls.
- Updates an on-screen state table showing queue contents, current distances, and visited arrays at each step.

### 3. Network Resilience & Fault Injection Lab
- Simulates real-world subway line closures or maintenance shutdowns.
- Clicking any station or track marks it as "Failed" (rendered in red with dashed cross-hatching).
- Automatically runs **Tarjan's Low-Link Algorithm** to identify:
  - **Articulation Stations**: Stations whose closure fragments the network into disconnected components.
  - **Bridges**: Rail tracks whose closure cuts off entire metro branches.

### 4. Station Management
- Operates entirely **in-memory** for fast, safe graph mutation:
  - Add new stations with custom IDs, names, and 2D canvas coordinates.
  - Connect stations with bidirectional tracks, custom transit weights (minutes), and line colors.
  - Delete existing tracks or stations with automatic graph topology updates.

---

## 3. Data Structures & Justification

| Data Structure | C++ Implementation | Usage in Project | Why This Was Chosen (Defense) |
| :--- | :--- | :--- | :--- |
| **Adjacency List** | `unordered_map<int, vector<AdjEdge>>` | Graph representation in `MetroGraph` | The metro system is a **sparse graph** ($V \approx 20, E \approx 25$). An adjacency matrix would consume $O(V^2)$ memory with mostly zero entries. An adjacency list takes only $O(V + E)$ memory and allows $O(\text{deg}(u))$ neighbor traversal. |
| **Priority Queue (Min-Heap)** | `std::priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>` | Dijkstra's Algorithm in `GraphAlgorithms` | A binary min-heap guarantees $O(\log V)$ retrieval of the unvisited station with the minimum tentative travel time, avoiding $O(V)$ linear scans. |
| **Indexed Vector Arrays** | `vector<int> dist(n, INT_MAX)`, `vector<bool> visited(n, false)` | State tracking in Dijkstra, BFS, DFS | Classical 1D vector arrays provide instant $O(1)$ random access by station ID, eliminating hash table overhead and matching standard college DSA paradigms. |
| **FIFO Vector Queue** | `vector<int> queue` with pointer `int head = 0` | Breadth-First Search (BFS) | A classical vector-based queue allows straightforward index tracking, inspection, and recording of frontier elements for the visualizer without requiring complex deque structures. |
| **Predecessor Map** | `vector<pair<int, int>> parent(n, {-1, -1})` | Backtracking path reconstruction | Records `parent[v] = {prevNode, edgeId}`. Using index-mapped pairs allows instantaneous backtrack reconstruction of the optimal route from destination back to source. |
| **Recursion / Call Stack** | Implicit program call stack | DFS and Tarjan's Algorithm | Uses standard recursion to traverse deeply into graph branches before backtracking. |

---

## 4. Algorithms & Complexity Analysis

### 1. Dijkstra's Algorithm (Minimum Travel Time)
- **Problem Formulation**: Given a directed/undirected graph with non-negative edge weights $w(u, v) \ge 0$, find the path with minimum total weight.
- **Mathematical Invariant**: When node $u$ is extracted from the min-heap, its tentative distance `dist[u]` is mathematically guaranteed to be the shortest distance from the source.
- **Step-by-Step Logic**:
  1. Initialize `dist[src] = 0` and all other `dist[v] = INT_MAX`.
  2. Push `(0, src)` into the min-priority queue.
  3. While priority queue is not empty:
     - Pop `(d, u)` with the smallest distance. If `visited[u]` is true, discard it.
     - Mark `visited[u] = true`. If $u = \text{destination}$, terminate early.
     - For each neighbor $v$ connected via edge $e$:
       $$\text{if } dist[u] + weight(e) < dist[v] \implies dist[v] = dist[u] + weight(e), \quad parent[v] = \{u, e.id\}$$
       Push `(dist[v], v)` into the priority queue.
- **Time Complexity**: $O((V + E) \log V)$ using a binary min-heap.
- **Space Complexity**: $O(V)$ for distance, visited, and parent vectors.

---

### 2. Breadth-First Search (Fewest Stops / Unweighted Shortest Path)
- **Problem Formulation**: Find a route minimizing the number of intermediate station hops regardless of track time.
- **Step-by-Step Logic**:
  1. Initialize `visited` array to `false` and push `src` into the queue. Mark `visited[src] = true`.
  2. Maintain `head = 0`. While `head < queue.size()`:
     - Pop `u = queue[head++]`.
     - If $u = \text{destination}$, break out.
     - For every unvisited neighbor $v$ of $u$:
       - Mark `visited[v] = true`.
       - Record `parent[v] = {u, edgeId}`.
       - Push $v$ to `queue`.
- **Time Complexity**: $O(V + E)$ — each vertex and edge is visited at most once.
- **Space Complexity**: $O(V)$ for queue and visited array.

---

### 3. Depth-First Search (Path Traversal)
- **Problem Formulation**: Explore network connectivity by traversing as deep as possible along each branch before backtracking.
- **Step-by-Step Logic**:
  1. Start at node $u$ and mark `visited[u] = true`.
  2. If $u == \text{target}$, return `true`.
  3. For every unvisited neighbor $v$, set `parent[v] = {u, edgeId}` and recursively call `dfsHelper(v)`.
  4. If a recursive call returns `true`, propagate `true` back up the call stack immediately.
- **Time Complexity**: $O(V + E)$.
- **Space Complexity**: $O(V)$ auxiliary recursion stack space in the worst case (linear chain).

---

### 4. Optimal Route Reconstruction (`reconstructRoute`)
- **Purpose**: Graph algorithms output predecessor pointers `parent[v] = {prevStation, edgeId}` rather than a full list of stations to keep intermediate operations $O(1)$. This function converts parent pointers into a forward itinerary.
- **Step-by-Step Logic**:
  1. **Corner Case**: If `src == dst`, return an immediate single-node itinerary.
  2. **Backtracking**: Start at `curr = dst`. Iteratively follow `parent[curr].first` and push both station and edge IDs into vectors until reaching `src`.
  3. **Vector Reversal**: Reverse the vectors in $O(L)$ linear time to transform the backwards path `[dst -> ... -> src]` into forward sequence `[src -> ... -> dst]`.
  4. **Metrics Accumulation**: Sum travel times along the edges and detect line transfers whenever adjacent edges belong to different metro lines (`edge[i]->line != edge[i-1]->line`).
- **Time Complexity**: $O(L)$ where $L \le V$ is the path length.
- **Space Complexity**: $O(L)$ to store the route.

---

### 5. Tarjan's Bridge & Articulation Point Algorithm
- **Purpose**: Detect vulnerability hotspots in the transit network without brute-force re-evaluation.
- **Key Concepts**:
  - `disc[u]`: Discovery timestamp of node $u$ in the DFS tree.
  - `low[u]`: Lowest discovery time reachable from $u$ via tree edges and back-edges.
- **Conditions**:
  - **Bridge Track $(u, v)$**: If $low[v] > disc[u]$, edge $(u, v)$ is a critical bridge. (No back-edge exists from $v$ or its descendants to $u$ or its ancestors).
  - **Articulation Station $u$**:
    - If $u$ is the DFS root and has $\ge 2$ children in the DFS tree.
    - If $u$ is not the root and has a child $v$ such that $low[v] \ge disc[u]$.
- **Time Complexity**: $O(V + E)$ — executes in a single linear DFS traversal.
- **Comparison**: Brute-force removal would take $O(E \times (V + E)) \approx O(E^2)$ time. Tarjan's approach reduces this to $O(V + E)$.

---

## 5. Qt GUI & Architecture Concepts

1. **Custom Vector Rendering (`QPainter`)**:
   - `MetroMapView` overrides `paintEvent()` rather than using generic HTML or third-party web views.
   - Implements mathematical scaling to maintain aspect ratio, anti-aliased geometry, line glow effects, and station label rendering.
2. **Geometric Hit-Testing**:
   - Station clicks: Calculated using Euclidean distance from mouse position to station coordinates $(x, y)$:
     $$\sqrt{(x_{mouse} - x)^2 + (y_{mouse} - y)^2} \le \text{radius}$$
   - Track clicks: Calculated using perpendicular point-to-line segment projection.
3. **Decoupled Animation Engine (`QTimer`)**:
   - The algorithm visualizer runs via a non-blocking `QTimer` at 500ms intervals.
   - At each timer tick, it increments the snapshot pointer `m_currentStep` and triggers a repaint (`update()`), keeping the user interface completely fluid without freezing the event loop.
4. **Signals and Slots**:
   - Completely decouples the user interaction layer from the computation layer. When a user clicks a station on `MetroMapView`, it emits `stationClicked(int stationId)`, which `MainWindow` handles and routes to the active view.

---

## 6. Viva & Examination Defense Questions

#### Q1: Why did you choose Dijkstra over BFS for fastest travel time?
> *"BFS only finds the shortest path in unweighted graphs where all edges have the same cost. In a real-world metro network, track segments have different travel times (e.g., 2 minutes between close stations, but 12 minutes between distant hubs). Dijkstra uses edge weights representing transit time in minutes and a priority queue to guarantee the globally minimum travel duration."*

#### Q2: How are transfers / interchanges detected?
> *"During route reconstruction, the system inspects the sequence of edge IDs traversed. Whenever two adjacent edges belong to different metro lines (e.g., transitioning from the Blue Line to the Yellow Line at Rajiv Chowk), the interchange counter is incremented and an explicit transfer instruction is appended to the user itinerary."*

#### Q3: Why is an Adjacency List better than an Adjacency Matrix for this system?
> *"A metro network is a sparse graph. For $V=18$ stations, an adjacency matrix allocates $18 \times 18 = 324$ cells, of which over 85% would store zero. An adjacency list only stores existing connections, using $O(V + E)$ memory and allowing neighbor iteration in $O(\text{deg}(u))$ time instead of checking $O(V)$ cells every time."*

#### Q4: How does the Algorithm Visualizer work without lagging or freezing the UI?
> *"We use an event-driven snapshot model. Before visualization starts, `VisualizerEngine` executes the algorithm once and captures an immutable sequence of `VisualizerStep` records (storing the current node, visited array, and queue state). A `QTimer` periodically updates the current step index at 500ms intervals and calls `update()`. Because the algorithm doesn't run inside the UI event loop, the interface never freezes."*

#### Q5: What makes Tarjan's algorithm efficient for resilience analysis?
> *"The naive brute-force method to find critical bridges is to remove each of the $E$ tracks one-by-one and run BFS to see if the graph is disconnected, which takes $O(E \times (V + E)) \approx O(E^2)$ time. Tarjan's algorithm discovers all bridges and articulation points in a single recursive DFS pass in $O(V + E)$ linear time using discovery times and low-link values."*

#### Q6: How does the Station Management feature work without a database?
> *"All station vertices, coordinates, and tracks are maintained directly in memory inside the `MetroGraph` instance. When an administrator adds a new station or track, the graph dynamically updates its internal adjacency list. The UI triggers recalculations and re-renders the visual canvas immediately, ensuring high performance with zero external database dependencies."*

---

## 7. Build & Execution Reference

### Recommended Qt Creator Build:
1. Open `MetroRouteSystem.pro` in **Qt Creator**.
2. Select **Desktop Qt 6 MinGW 64-bit**.
3. Press `Ctrl + R` to compile and launch.

### Command Line Build (MinGW):
```powershell
$env:PATH = "C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"
qmake MetroRouteSystem.pro
mingw32-make release
.\release\MetroRouteSystem.exe
```
