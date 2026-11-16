#ifndef METROMAPVIEW_H
#define METROMAPVIEW_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <unordered_set>
#include "VisualizerEngine.h"
#include "MetroGraph.h"

class MetroMapView : public QWidget {
    Q_OBJECT

public:
    enum ViewMode {
        ViewNavigator,
        ViewVisualizer,
        ViewResilience,
        ViewManagement
    };

    explicit MetroMapView(QWidget* parent = nullptr);

    void setGraph(MetroGraph* graph);
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return m_viewMode; }

    void setSourceStation(int id);
    void setDestinationStation(int id);
    void setSelectedStation(int id);
    void setManagementLink(int a, int b);
    void setShowCritical(bool show);

    int getSourceStation() const { return m_srcStation; }
    int getDestinationStation() const { return m_dstStation; }
    int getSelectedStation() const { return m_selectedStation; }

    void setActiveRoute(const RouteResult& route, const QString& algColor = "#2ecc71");
    void setVisualizerStep(const VisualizerStep* step);
    void clearRoute();

signals:
    void stationClicked(int stationId);
    void edgeClicked(int edgeId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    MetroGraph* m_graph = nullptr;
    ViewMode m_viewMode = ViewNavigator;

    int m_srcStation = 0;
    int m_dstStation = 4;
    int m_selectedStation = -1;
    int m_linkA = -1;
    int m_linkB = -1;
    bool m_showCritical = false;

    std::unordered_set<int> m_routeEdges;
    std::unordered_set<int> m_routeNodes;
    QColor m_routeColor;
    const VisualizerStep* m_currentStep = nullptr;

    const double MAP_WIDTH = 1080.0;
    const double MAP_HEIGHT = 640.0;

    QPointF mapToScreen(const QPointF& mapPt) const;
    QPointF screenToMap(const QPointF& screenPt) const;
    double scaleFactor() const;

    int findStationAt(const QPointF& mapPt) const;
    int findEdgeAt(const QPointF& mapPt) const;
    double distanceToSegment(const QPointF& p, const QPointF& a, const QPointF& b) const;
};

#endif // METROMAPVIEW_H
