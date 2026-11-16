#include "MainWindow.h"
#include "AppTheme.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

using namespace std;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Metro Route Navigation System - C++ & Qt");
    resize(1280, 760);

    setupUI();
    setStyleSheet(AppTheme::getStyleSheet());

    m_mapView->setGraph(&m_graph);
    m_mapView->setSourceStation(0);
    m_mapView->setDestinationStation(4);

    m_navPage->populateStationCombos();
    m_visPage->populateStationCombos();
    m_resPage->populateStationCombos();
    m_mgmtPage->populateStationCombos();

    switchView(MetroMapView::ViewNavigator);
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget* sidebar = new QWidget(this);
    sidebar->setFixedWidth(200);
    sidebar->setObjectName("sidebar");
    QVBoxLayout* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(0);

    QLabel* titleLabel = new QLabel("Metro Route\nNavigation", sidebar);
    titleLabel->setObjectName("appTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    sideLayout->addWidget(titleLabel);

    auto makeBtn = [this, sidebar, sideLayout](const QString& text, MetroMapView::ViewMode mode) -> QPushButton* {
        QPushButton* b = new QPushButton(text, sidebar);
        b->setObjectName("navButton");
        sideLayout->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, mode]() { switchView(mode); });
        return b;
    };
    m_btnNav  = makeBtn("Metro Navigator", MetroMapView::ViewNavigator);
    m_btnVis  = makeBtn("Algorithm Visualizer", MetroMapView::ViewVisualizer);
    m_btnRes  = makeBtn("Network Resilience Lab", MetroMapView::ViewResilience);
    m_btnMgmt = makeBtn("Station Management", MetroMapView::ViewManagement);
    sideLayout->addStretch();
    mainLayout->addWidget(sidebar);

    QWidget* centerContainer = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    m_mapView = new MetroMapView(this);
    centerLayout->addWidget(m_mapView, 1);
    m_hintBar = new QLabel(this);
    m_hintBar->setObjectName("hintBar");
    m_hintBar->setWordWrap(true);
    centerLayout->addWidget(m_hintBar);
    connect(m_mapView, &MetroMapView::stationClicked, this, &MainWindow::onStationClicked);
    connect(m_mapView, &MetroMapView::edgeClicked, this, &MainWindow::onEdgeClicked);
    mainLayout->addWidget(centerContainer, 1);

    QScrollArea* panelScroll = new QScrollArea(this);
    panelScroll->setFixedWidth(380);
    panelScroll->setWidgetResizable(true);
    panelScroll->setObjectName("panelScroll");

    m_panelStack = new QStackedWidget(panelScroll);
    m_panelStack->setObjectName("rightPanel");

    m_navPage = new NavigatorPage(&m_graph, m_mapView, this);
    m_panelStack->addWidget(m_navPage);

    m_visPage = new VisualizerPage(&m_graph, m_mapView, this);
    m_panelStack->addWidget(m_visPage);

    m_resPage = new ResiliencePage(&m_graph, m_mapView, this);
    m_panelStack->addWidget(m_resPage);

    m_mgmtPage = new ManagementPage(&m_graph, m_mapView, this);
    connect(m_mgmtPage, &ManagementPage::networkModified, this, &MainWindow::onNetworkModified);
    m_panelStack->addWidget(m_mgmtPage);

    panelScroll->setWidget(m_panelStack);
    mainLayout->addWidget(panelScroll);
}

void MainWindow::switchView(MetroMapView::ViewMode mode) {
    m_mapView->setViewMode(mode);
    const std::pair<QPushButton*, MetroMapView::ViewMode> navBtns[] = {
        {m_btnNav, MetroMapView::ViewNavigator},
        {m_btnVis, MetroMapView::ViewVisualizer},
        {m_btnRes, MetroMapView::ViewResilience},
        {m_btnMgmt, MetroMapView::ViewManagement}
    };
    for (const auto& pair : navBtns) {
        pair.first->setProperty("class", mode == pair.second ? "active" : "");
        pair.first->style()->unpolish(pair.first);
        pair.first->style()->polish(pair.first);
    }
    if (mode != MetroMapView::ViewVisualizer) m_visPage->pauseSimulation();

    switch (mode) {
    case MetroMapView::ViewNavigator:
        m_panelStack->setCurrentWidget(m_navPage);
        m_hintBar->setText("Hint: Click two stations on the map or use dropdowns. A = Source, B = Destination.");
        m_navPage->updateRoute(); break;
    case MetroMapView::ViewVisualizer:
        m_panelStack->setCurrentWidget(m_visPage);
        m_hintBar->setText("Hint: Green = Visited, Cyan = Current Node, White = Relaxed Edges. Watch how queue/stack changes!");
        m_visPage->resetSimulation(); break;
    case MetroMapView::ViewResilience:
        m_panelStack->setCurrentWidget(m_resPage);
        m_hintBar->setText("Hint: Click any station or track on the map to toggle failure! Orange indicates critical bridges/articulation points.");
        m_resPage->updateResilience(); break;
    case MetroMapView::ViewManagement:
        m_panelStack->setCurrentWidget(m_mgmtPage);
        m_hintBar->setText("Hint: Click two stations on the map to select From (A) and To (B) for adding a connection.");
        m_mgmtPage->updateManagement(); break;
    }
}

void MainWindow::onStationClicked(int sid) {
    auto mode = m_mapView->getViewMode();
    if (mode == MetroMapView::ViewResilience) {
        m_graph.setStationFailed(sid, !m_graph.isStationFailed(sid));
        m_resPage->updateResilience();
        m_mapView->update();
        return;
    }
    if (mode == MetroMapView::ViewManagement) {
        m_mgmtPage->handleMapStationClick(sid);
        return;
    }

    m_mapView->setSelectedStation(sid);
    if (mode == MetroMapView::ViewNavigator || mode == MetroMapView::ViewVisualizer) {
        int src = m_mapView->getSourceStation(), dst = m_mapView->getDestinationStation();
        if (src == -1 || dst != -1) {
            m_mapView->setSourceStation(sid);
            m_mapView->setDestinationStation(-1);
        } else if (sid != src) {
            m_mapView->setDestinationStation(sid);
        }

        if (mode == MetroMapView::ViewNavigator) {
            m_navPage->updateRoute();
            m_navPage->showStationInfo(sid);
        } else {
            m_visPage->resetSimulation();
        }
    }
}

void MainWindow::onEdgeClicked(int eid) {
    if (m_mapView->getViewMode() == MetroMapView::ViewResilience) {
        m_graph.setEdgeFailed(eid, !m_graph.isEdgeFailed(eid));
        m_resPage->updateResilience();
        m_mapView->update();
    }
}

void MainWindow::onNetworkModified() {
    m_navPage->populateStationCombos();
    m_visPage->populateStationCombos();
    m_resPage->populateStationCombos();
    m_mgmtPage->populateStationCombos();
    m_mapView->update();
}
