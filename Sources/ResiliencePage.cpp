#include "ResiliencePage.h"
#include "AppTheme.h"
#include "GraphAlgorithms.h"
#include "ResilienceEngine.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

using namespace std;

ResiliencePage::ResiliencePage(MetroGraph *graph, MetroMapView *mapView,
                               QWidget *parent)
    : QWidget(parent), m_graph(graph), m_mapView(mapView) {
  setupUI();
}

void ResiliencePage::setupUI() {
  QVBoxLayout *l = new QVBoxLayout(this);
  l->setContentsMargins(16, 16, 16, 16);
  l->setSpacing(8);

  l->addWidget(new QLabel("<h2>Network Simulator</h2>", this));

  QHBoxLayout *topBtns = new QHBoxLayout();
  m_btnToggleCritical = new QPushButton("Show Critical Points", this);
  m_btnToggleCritical->setCheckable(true);
  m_btnRestoreNetwork = new QPushButton("Restore Network", this);
  topBtns->addWidget(m_btnToggleCritical);
  topBtns->addWidget(m_btnRestoreNetwork);
  l->addLayout(topBtns);

  m_impactLabel = new QLabel(this);
  m_impactLabel->setWordWrap(true);
  l->addWidget(m_impactLabel);

  l->addWidget(new QLabel("<b>Active Failures</b>", this));
  m_failuresTable = new QTableWidget(0, 2, this);
  m_failuresTable->setHorizontalHeaderLabels({"Component", "Action"});
  m_failuresTable->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Stretch);
  m_failuresTable->horizontalHeader()->setSectionResizeMode(1,
                                                            QHeaderView::Fixed);
  m_failuresTable->setColumnWidth(1, 95);
  AppTheme::styleTable(m_failuresTable);
  m_failuresTable->setFixedHeight(0);
  l->addWidget(m_failuresTable);

  l->addWidget(new QLabel("<b>Journey Metrics</b>", this));
  l->addWidget(new QLabel("Source:", this));
  m_srcCombo = new QComboBox(this);
  l->addWidget(m_srcCombo);

  l->addWidget(new QLabel("Destination:", this));
  m_dstCombo = new QComboBox(this);
  l->addWidget(m_dstCombo);

  QHBoxLayout *algLayout = new QHBoxLayout();
  m_btnDijkstra = new QPushButton("Fastest", this);
  m_btnBFS = new QPushButton("Fewest stops", this);
  m_btnDijkstra->setCheckable(true);
  m_btnBFS->setCheckable(true);
  m_btnDijkstra->setChecked(true);
  algLayout->addWidget(m_btnDijkstra);
  algLayout->addWidget(m_btnBFS);
  l->addLayout(algLayout);

  m_compareTable = new QTableWidget(3, 2, this);
  m_compareTable->setHorizontalHeaderLabels({"Metric", "Value"});
  m_compareTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  AppTheme::styleTable(m_compareTable);
  m_compareTable->setFixedHeight(120);
  l->addWidget(m_compareTable);

  m_routeLabel = new QLabel(this);
  m_routeLabel->setWordWrap(true);
  l->addWidget(m_routeLabel);
  l->addStretch();

  connect(m_btnToggleCritical, &QPushButton::clicked, this,
          &ResiliencePage::onToggleCriticalClicked);
  connect(m_btnRestoreNetwork, &QPushButton::clicked, this,
          &ResiliencePage::onRestoreNetworkClicked);
  connect(m_srcCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ResiliencePage::onSrcChanged);
  connect(m_dstCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ResiliencePage::onDstChanged);
  connect(m_btnDijkstra, &QPushButton::clicked, this,
          [this]() { onAlgSelected(true); });
  connect(m_btnBFS, &QPushButton::clicked, this,
          [this]() { onAlgSelected(false); });
}

void ResiliencePage::populateStationCombos() {
  if (!m_graph)
    return;
  auto sorted = m_graph->getSortedStations();
  AppTheme::populateStationCombo(m_srcCombo, sorted);
  AppTheme::populateStationCombo(m_dstCombo, sorted);
  int i0 = m_srcCombo->findData(0), i4 = m_dstCombo->findData(4);
  if (i0 != -1)
    m_srcCombo->setCurrentIndex(i0);
  if (i4 != -1)
    m_dstCombo->setCurrentIndex(i4);
}

void ResiliencePage::onToggleCriticalClicked() {
  m_showCritical = !m_showCritical;
  m_btnToggleCritical->setChecked(m_showCritical);
  if (m_mapView)
    m_mapView->setShowCritical(m_showCritical);
  updateResilience();
}

void ResiliencePage::onRestoreNetworkClicked() {
  if (m_graph)
    m_graph->restoreAll();
  updateResilience();
  if (m_mapView)
    m_mapView->update();
}

void ResiliencePage::onSrcChanged(int idx) {
  if (idx >= 0 && m_mapView) {
    m_mapView->setSourceStation(m_srcCombo->itemData(idx).toInt());
    updateResilience();
  }
}

void ResiliencePage::onDstChanged(int idx) {
  if (idx >= 0 && m_mapView) {
    m_mapView->setDestinationStation(m_dstCombo->itemData(idx).toInt());
    updateResilience();
  }
}

void ResiliencePage::onAlgSelected(bool isDijkstra) {
  m_useDijkstra = isDijkstra;
  m_btnDijkstra->setChecked(isDijkstra);
  m_btnBFS->setChecked(!isDijkstra);
  updateResilience();
}

void ResiliencePage::updateResilience() {
  if (!m_graph || !m_mapView)
    return;
  auto a = ResilienceEngine::analyze(*m_graph);

  QString html =
      QString("<p><b>Active Stations:</b> %1 of %2</p><p><b>Connected "
              "Sub-networks:</b> %3</p><p><b>Network Connected:</b> "
              "%4</p><p><b>Cut-off Stations:</b> %5</p>")
          .arg(a.totalActiveStations)
          .arg(m_graph->getStations().size())
          .arg(a.connectedComponents)
          .arg(a.connectedComponents == 1
                   ? "<span style='color:#2ecc71;'><b>Yes</b></span>"
                   : "<span style='color:#e5484d;'><b>No</b></span>")
          .arg(a.cutOffStations);
  if (m_showCritical) {
    html += QString("<p><b>Critical Stations:</b> %1</p><p><b>Critical "
                    "Tracks:</b> %2</p>")
                .arg(a.articulationStations.size())
                .arg(a.bridgeEdges.size());
  }
  m_impactLabel->setText(html);

  const auto &fSt = m_graph->getFailedStations(),
             &fEd = m_graph->getFailedEdges();
  int totalF = static_cast<int>(fSt.size() + fEd.size());
  m_failuresTable->setRowCount(totalF);
  int row = 0;

  auto addFailRow = [this, &row](const QString &title, auto unFail) {
    m_failuresTable->setItem(row, 0, new QTableWidgetItem(title));
    QPushButton *btn = new QPushButton("Restore");
    connect(btn, &QPushButton::clicked, this, [this, unFail]() {
      unFail();
      updateResilience();
      m_mapView->update();
    });
    m_failuresTable->setCellWidget(row, 1, btn);
    m_failuresTable->setRowHeight(row, 36);
    row++;
  };

  for (int sid : fSt) {
    const Station *s = m_graph->getStation(sid);
    addFailRow("Station: " +
                   (s ? QString::fromStdString(s->name) : QString::number(sid)),
               [this, sid]() { m_graph->setStationFailed(sid, false); });
  }
  for (int eid : fEd) {
    const Edge *e = m_graph->getEdge(eid);
    const Station *sa = e ? m_graph->getStation(e->u) : nullptr,
                  *sb = e ? m_graph->getStation(e->v) : nullptr;
    addFailRow(QString("Track: %1 - %2")
                   .arg(sa ? QString::fromStdString(sa->name) : "?")
                   .arg(sb ? QString::fromStdString(sb->name) : "?"),
               [this, eid]() { m_graph->setEdgeFailed(eid, false); });
  }

  int fh = std::max(26, m_failuresTable->horizontalHeader()->height());
  m_failuresTable->setFixedHeight(
      totalF == 0 ? 0
                  : fh + totalF * 36 + m_failuresTable->frameWidth() * 2 + 4);

  int src = m_mapView->getSourceStation(),
      dst = m_mapView->getDestinationStation();
  if (src != -1 && dst != -1) {
    RouteResult now =
        m_useDijkstra
            ? GraphAlgorithms::findFastestRouteDijkstra(*m_graph, src, dst,
                                                        false)
            : GraphAlgorithms::findFewestStopsBFS(*m_graph, src, dst, false);
    m_compareTable->setItem(0, 0, new QTableWidgetItem("Minutes"));
    m_compareTable->setItem(
        0, 1,
        new QTableWidgetItem(now.found ? QString("%1 m").arg(now.totalTime)
                                       : "Blocked"));
    m_compareTable->setItem(1, 0, new QTableWidgetItem("Stops"));
    m_compareTable->setItem(
        1, 1,
        new QTableWidgetItem(now.found ? QString::number(now.totalStops)
                                       : "Blocked"));
    m_compareTable->setItem(2, 0, new QTableWidgetItem("Interchanges"));
    m_compareTable->setItem(
        2, 1,
        new QTableWidgetItem(now.found ? QString::number(now.interchanges)
                                       : "Blocked"));

    for (int r = 0; r < 3; ++r)
      m_compareTable->setRowHeight(r, 28);
    int ch = std::max(26, m_compareTable->horizontalHeader()->height());
    m_compareTable->setFixedHeight(ch + 3 * 28 +
                                   m_compareTable->frameWidth() * 2 + 4);

    if (!now.found) {
      m_routeLabel->setText("<p style='color:#e5484d;'><b>No route available "
                            "with current failures!</b></p>");
      m_mapView->clearRoute();
    } else {
      QString pathStr = "<b>Route:</b> ";
      for (size_t i = 0; i < now.stationPath.size(); ++i) {
        const Station *st = m_graph->getStation(now.stationPath[i]);
        pathStr += (i > 0 ? " -> " : "") +
                   (st ? QString::fromStdString(st->name) : "?");
      }
      m_routeLabel->setText(pathStr);
      m_mapView->setActiveRoute(now, fSt.empty() && fEd.empty()
                                         ? AppTheme::SuccessGreen
                                         : AppTheme::Cyan);
    }
  }
}
