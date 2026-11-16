#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include "MetroGraph.h"
#include "MetroMapView.h"
#include "NavigatorPage.h"
#include "VisualizerPage.h"
#include "ResiliencePage.h"
#include "ManagementPage.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void switchView(MetroMapView::ViewMode mode);
    void onStationClicked(int stationId);
    void onEdgeClicked(int edgeId);
    void onNetworkModified();

private:
    void setupUI();

    MetroGraph m_graph;

    MetroMapView* m_mapView = nullptr;
    QLabel* m_hintBar = nullptr;
    QStackedWidget* m_panelStack = nullptr;

    QPushButton* m_btnNav = nullptr;
    QPushButton* m_btnVis = nullptr;
    QPushButton* m_btnRes = nullptr;
    QPushButton* m_btnMgmt = nullptr;

    NavigatorPage* m_navPage = nullptr;
    VisualizerPage* m_visPage = nullptr;
    ResiliencePage* m_resPage = nullptr;
    ManagementPage* m_mgmtPage = nullptr;
};

#endif // MAINWINDOW_H
