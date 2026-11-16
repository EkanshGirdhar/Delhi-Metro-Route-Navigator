#ifndef NAVIGATORPAGE_H
#define NAVIGATORPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "MetroGraph.h"
#include "MetroMapView.h"

class RouteTimelineWidget : public QWidget {
    Q_OBJECT
public:
    explicit RouteTimelineWidget(QWidget* parent = nullptr);
    void setRoute(const RouteResult& route, const MetroGraph* graph);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    RouteResult m_route;
    const MetroGraph* m_graph = nullptr;
};

class NavigatorPage : public QWidget {
    Q_OBJECT

public:
    explicit NavigatorPage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent = nullptr);

    void populateStationCombos();
    void updateRoute();
    void showStationInfo(int stationId);

signals:
    void routeUpdated();

private slots:
    void onSrcChanged(int index);
    void onDstChanged(int index);
    void onSwapClicked();
    void onAlgSelected(bool isDijkstra);

private:
    void setupUI();

    MetroGraph* m_graph = nullptr;
    MetroMapView* m_mapView = nullptr;
    bool m_useDijkstra = true;
    int m_inspectedStationId = -1;

    QComboBox* m_srcCombo = nullptr;
    QComboBox* m_dstCombo = nullptr;
    QPushButton* m_btnSwap = nullptr;
    QPushButton* m_btnDijkstra = nullptr;
    QPushButton* m_btnBFS = nullptr;
    QLabel* m_statsLabel = nullptr;
    RouteTimelineWidget* m_timelineWidget = nullptr;
    QLabel* m_stationInfoLabel = nullptr;
};

#endif // NAVIGATORPAGE_H
