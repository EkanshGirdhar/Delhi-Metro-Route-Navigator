<div align="center">

# Delhi Metro Route Navigation & Graph Analysis System

An interactive desktop application built with **C++17** and **Qt 6** modeling a metropolitan transit network as a weighted undirected graph. Features shortest-path routing, real-time algorithm visualization, network fault resilience, and dynamic in-memory graph management.

[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Qt 6](https://img.shields.io/badge/GUI_Framework-Qt_6-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-0078D6)](https://github.com)
[![Build](https://img.shields.io/badge/Build-QMake%20%7C%20MinGW-blueviolet)](https://github.com)
[![Status](https://img.shields.io/badge/Status-Completed-success)](https://github.com)

[Explore Features](#-core-features) • [Algorithm Details](#-algorithms--complexity) • [Installation](#-getting-started) • [Technical Defense Guide](PROJECT_EXPLANATION_GUIDE.md)

</div>

---

## Table of Contents
- [About The Project](#-about-the-project)
- [Core Features](#-core-features)
- [Algorithms & Complexity](#-algorithms--complexity)
- [System Architecture](#-system-architecture)
- [Project Directory Structure](#-project-directory-structure)
- [Getting Started](#-getting-started)
  - [Prerequisites](#prerequisites)
  - [Option A: Build via Qt Creator (Recommended)](#option-a-build-via-qt-creator-recommended)
  - [Option B: Build via Command Line](#option-b-build-via-command-line)
- [Academic Context](#-academic-context)

---

## About The Project

Metropolitan transit networks transport millions of commuters daily. Efficient navigation requires solving classic graph theory problems: finding the fastest transit path, minimizing line transfers, and identifying vulnerable tracks prone to single-point failures.

A desktop application written in C++17 using the Qt 6 framework that models the Delhi Metro transit network as a weighted undirected graph. The project implements shortest path algorithms, interactive step-by-step algorithm visualization, network fault resilience analysis, and in-memory station network editing.

---

## ✨ Core Features

### 1. Metro Route Navigator
- **Dijkstra's Algorithm**: Calculates the fastest route minimizing overall travel time in minutes.
- **Breadth-First Search (BFS)**: Computes the optimal route minimizing the number of intermediate station stops and line transfers.
- **Turn-by-Turn Itinerary**: Generates detailed travel instructions highlighting line interchange stations and estimated transit duration.
- **Interactive Map Highlighting**: Dynamically illuminates the chosen path with glowing line tracks on the vector map.

### 2. Algorithm Visualizer
- **Real-Time Step-by-Step Playback**: Watch graph traversal algorithms run live on the metro grid:
  - **Dijkstra**: Observe node distance relaxation and priority queue extraction.
  - **BFS**: View level-by-level frontier exploration using a FIFO queue.
  - **DFS**: Trace deep branch exploration and recursive backtracking.
- **Node State Color Palette**: Color-coded states for nodes:
  - ⚪ `Unvisited` &bull; 🟠 `Frontier / In Queue` &bull; 🔵 `Currently Visiting` &bull; 🟢 `Finalized / Settled`
- **Playback Controls**: Step Forward, Play/Pause at a steady 500ms interval, and Reset with live state tables.

### 3. Network Resilience & Vulnerability Lab
- **Single Point of Failure (SPOF) Analysis**: Runs **Tarjan's Low-Link Algorithm** in $O(V + E)$ linear time to detect:
  - **Articulation Stations**: Stations whose closure splits the transit system into disconnected sub-networks.
  - **Bridge Tracks**: Critical railway links whose disruption isolates entire metro segments.
- **Interactive Fault Injection**: Click any station or track to simulate closures and evaluate rerouting feasibility in real time.

### 4. In-Memory Station Management
- **Dynamic Topology Mutation**:
  - Add new stations with custom coordinates.
  - Connect stations with custom travel weights (minutes) and line color assignments.
  - Remove existing operational tracks or decommission stations.
- Runs **100% in-memory** with immediate topological updates and zero external database dependencies.

### 5. Custom 2D Vector Map Canvas
- Built from the ground up using Qt's `QPainter`.
- Anti-aliased line rendering, letterboxed scaling for any display resolution, and geometric hit-testing (point-to-circle for stations and point-to-segment projection for tracks).

---

## Algorithms & Complexity

| Algorithm | Objective | Primary Data Structure | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Dijkstra's Algorithm** | Fastest Travel Time | `std::priority_queue` (Min-Heap) + 1D `vector` | $\mathcal{O}((V + E) \log V)$ | $\mathcal{O}(V + E)$ |
| **Breadth-First Search (BFS)** | Fewest Station Stops | Vector-based FIFO Queue + `vector<bool>` | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |
| **Depth-First Search (DFS)** | Deep Path Traversal | Program Call Stack + `vector<bool>` | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |
| **Tarjan's Low-Link** | Bridges & Articulation Points | Discovery & Low-link Arrays + DFS | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |
| **Route Reconstruction** | Predecessor Backtracking | `vector<pair<int, int>>` parent pointers | $\mathcal{O}(L)$ ($L \le V$) | $\mathcal{O}(L)$ |

> For in-depth mathematical formulas, invariants, and college viva questions, see the [Project Explanation Guide](PROJECT_EXPLANATION_GUIDE.md).

---

## 🏛️ System Architecture

The application adopts a modular, decoupled three-layer architecture:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        1. Presentation Layer (Qt 6)                    │
│   MainWindow      • MetroMapView (QPainter)     • NavigatorPage        │
│   VisualizerPage  • ResiliencePage              • ManagementPage       │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
┌───────────────────────────────────▼────────────────────────────────────┐
│                    2. Algorithm & Computation Layer                    │
│   GraphAlgorithms (Dijkstra, BFS, DFS) • VisualizerEngine              │
│   ResilienceEngine (Tarjan's Bridges & Articulation Points)            │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
┌───────────────────────────────────▼────────────────────────────────────┐
│                       3. Graph Data Model Layer                        │
│   MetroGraph (In-memory stations, edges, adjacency list, topology)     │
└────────────────────────────────────────────────────────────────────────┘
```

---

## Project Directory Structure

```text
Metro-Route-System/
├── AppTheme.h                      # Application-wide styling, fonts, and dark palette
├── GraphAlgorithms.h / .cpp        # Dijkstra, BFS, DFS, and route reconstruction
├── MainWindow.h / .cpp             # Main GUI shell and sidebar coordinator
├── ManagementPage.h / .cpp         # In-memory station/connection management tab
├── MetroGraph.h / .cpp             # Graph representation and adjacency list
├── MetroMapView.h / .cpp           # Interactive 2D vector canvas (QPainter)
├── MetroRouteSystem.pro            # QMake configuration file
├── NavigatorPage.h / .cpp          # Route finder and itinerary presentation
├── ResilienceEngine.h / .cpp       # Tarjan's bridge and articulation point detector
├── ResiliencePage.h / .cpp         # Fault injection and resilience analysis tab
├── VisualizerEngine.h / .cpp       # Step-by-step algorithm snapshot recorder
├── VisualizerPage.h / .cpp         # Playback controls and state table panel
├── main.cpp                        # Application entry point
├── .gitignore                      # Git ignore rules for Qt/MinGW builds
├── README.md                       # Repository overview and documentation
└── PROJECT_EXPLANATION_GUIDE.md    # Technical viva and theory defense guide
```

---

## 🚀 Getting Started

### Prerequisites
- **C++17 Compiler**: GCC / MinGW 11.2+ or Clang 13+
- **Qt 6 Framework**: Qt 6.2 LTS or newer (tested on Qt 6.11 MinGW 64-bit)

---

### Option A: Build via Qt Creator (Recommended)

1. Clone or download this repository:
   ```bash
   git clone https://github.com/<your-username>/Metro-Route-Navigation-System.git
   cd Metro-Route-Navigation-System
   ```
2. Open **Qt Creator**.
3. Click **File > Open File or Project** and select `MetroRouteSystem.pro`.
4. Configure the project with your installed **Desktop Qt 6 MinGW 64-bit** kit.
5. Click **Run** (`Ctrl + R`) to compile and launch.

---

### Option B: Build via Command Line (PowerShell / Bash)

1. Open a terminal in the project directory.
2. Ensure Qt 6 and MinGW compiler tools are on your `PATH`:
   ```powershell
   $env:PATH = "C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"
   ```
3. Generate Makefiles and build:
   ```powershell
   qmake MetroRouteSystem.pro
   mingw32-make release
   ```
4. Run the executable:
   ```powershell
   .\release\MetroRouteSystem.exe
   ```

---

<div align="center">
⭐ If you found this repository useful, please consider giving it a star!
</div>
