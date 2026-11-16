#include "NavigatorPage.h"
#include "GraphAlgorithms.h"
#include "AppTheme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <algorithm>

using namespace std;

RouteTimelineWidget::RouteTimelineWidget(QWidget* parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(0);
}

void RouteTimelineWidget::setRoute(const RouteResult& route, const MetroGraph* graph) {
    m_route = route;
    m_graph = graph;
    if (!m_route.found || m_route.stationPath.empty()) {
        setFixedHeight(0);
    } else {
        int totalHeight = 16;
        for (size_t i = 0; i < m_route.stationPath.size(); ++i) {
            bool transfer = false;
            if (i > 0 && i < m_route.edgePath.size() && m_graph) {
                const Edge* e1 = m_graph->getEdge(m_route.edgePath[i - 1]);
                const Edge* e2 = m_graph->getEdge(m_route.edgePath[i]);
                if (e1 && e2 && e1->line != e2->line) transfer = true;
            }
            totalHeight += transfer ? 46 : 32;
        }
        setFixedHeight(totalHeight + 10);
    }
    update();
}

void RouteTimelineWidget::paintEvent(QPaintEvent*) {
    if (!m_graph || !m_route.found || m_route.stationPath.empty()) return;
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    int count = static_cast<int>(m_route.stationPath.size());
    int x = 16, y = 16;

    for (int i = 0; i < count; ++i) {
        int sid = m_route.stationPath[i];
        const Station* st = m_graph->getStation(sid);
        QString name = st ? QString::fromStdString(st->name) : QString::number(sid);

        int edgeIdx = std::min(i, static_cast<int>(m_route.edgePath.size()) - 1);
        QColor dotColor(AppTheme::LineOrange);
        if (edgeIdx >= 0 && edgeIdx < static_cast<int>(m_route.edgePath.size())) {
            const Edge* e = m_graph->getEdge(m_route.edgePath[edgeIdx]);
            if (e) dotColor = AppTheme::getLineColor(e->line);
        }

        bool transfer = false;
        QString tText;
        if (i > 0 && i < static_cast<int>(m_route.edgePath.size())) {
            const Edge* e1 = m_graph->getEdge(m_route.edgePath[i - 1]);
            const Edge* e2 = m_graph->getEdge(m_route.edgePath[i]);
            if (e1 && e2 && e1->line != e2->line) {
                transfer = true;
                tText = "Change to " + QString::fromStdString(e2->line) + " line";
            }
        }

        int step = transfer ? 46 : 32;
        if (i < count - 1) {
            p.setPen(QPen(QColor(AppTheme::BorderDark), 2.5));
            p.drawLine(x, y, x, y + step);
        }

        p.setPen(QPen(QColor("#1e2636"), 2.0));
        p.setBrush(dotColor);
        p.drawEllipse(QPointF(x, y), 5.5, 5.5);

        p.setFont(QFont("Segoe UI", 11, QFont::DemiBold));
        p.setPen(QColor(AppTheme::TextLight));
        p.drawText(x + 18, y + 5, name);

        if (transfer) {
            p.setFont(QFont("Segoe UI", 9));
            p.setPen(QColor(AppTheme::TextMuted));
            p.drawText(x + 18, y + 22, tText);
        }
        y += step;
    }
}

NavigatorPage::NavigatorPage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent)
    : QWidget(parent), m_graph(graph), m_mapView(mapView) {
    setupUI();
}

void NavigatorPage::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    layout->addWidget(new QLabel("<h2 style='margin:0 0 6px; font-size:18px; font-weight:700; color:#eef1f6;'>Metro Navigator</h2>", this));

    auto addField = [this, layout](const QString& text, QComboBox*& combo) {
        QLabel* lbl = new QLabel(text, this);
        lbl->setStyleSheet("color: #b3bdce; font-size: 13px; margin-top: 4px;");
        layout->addWidget(lbl);
        combo = new QComboBox(this);
        layout->addWidget(combo);
    };
    addField("Source", m_srcCombo);
    addField("Destination", m_dstCombo);

    m_btnSwap = new QPushButton("Swap", this);
    m_btnSwap->setFixedHeight(32);
    layout->addWidget(m_btnSwap);

    QHBoxLayout* algLayout = new QHBoxLayout();
    algLayout->setSpacing(0);
    algLayout->setContentsMargins(0, 4, 0, 4);

    m_btnDijkstra = new QPushButton("Fastest (Dijkstra)", this);
    m_btnBFS = new QPushButton("Fewest stops (BFS)", this);
    m_btnDijkstra->setObjectName("segBtnLeft");
    m_btnBFS->setObjectName("segBtnRight");
    m_btnDijkstra->setCheckable(true);
    m_btnBFS->setCheckable(true);
    m_btnDijkstra->setChecked(true);
    m_btnDijkstra->setFixedHeight(32);
    m_btnBFS->setFixedHeight(32);
    algLayout->addWidget(m_btnDijkstra);
    algLayout->addWidget(m_btnBFS);
    layout->addLayout(algLayout);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setTextFormat(Qt::RichText);
    m_statsLabel->setWordWrap(true);
    layout->addWidget(m_statsLabel);

    m_timelineWidget = new RouteTimelineWidget(this);
    layout->addWidget(m_timelineWidget);

    m_stationInfoLabel = new QLabel(this);
    m_stationInfoLabel->setTextFormat(Qt::RichText);
    m_stationInfoLabel->setWordWrap(true);
    layout->addWidget(m_stationInfoLabel);

    layout->addStretch();

    connect(m_srcCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NavigatorPage::onSrcChanged);
    connect(m_dstCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NavigatorPage::onDstChanged);
    connect(m_btnSwap, &QPushButton::clicked, this, &NavigatorPage::onSwapClicked);
    connect(m_btnDijkstra, &QPushButton::clicked, this, [this]() { onAlgSelected(true); });
    connect(m_btnBFS,      &QPushButton::clicked, this, [this]() { onAlgSelected(false); });
}

void NavigatorPage::populateStationCombos() {
    if (!m_graph) return;
    auto sorted = m_graph->getSortedStations();
    AppTheme::populateStationCombo(m_srcCombo, sorted);
    AppTheme::populateStationCombo(m_dstCombo, sorted);

    int idxK = m_srcCombo->findData(13); // Kalkaji
    int idxJ = m_dstCombo->findData(1);  // Janakpuri
    if (idxK != -1) m_srcCombo->setCurrentIndex(idxK);
    if (idxJ != -1) m_dstCombo->setCurrentIndex(idxJ);
}

void NavigatorPage::onSrcChanged(int index) {
    if (index >= 0 && m_mapView) {
        int id = m_srcCombo->itemData(index).toInt();
        m_mapView->setSourceStation(id);
        m_inspectedStationId = id;
        updateRoute();
    }
}

void NavigatorPage::onDstChanged(int index) {
    if (index >= 0 && m_mapView) {
        m_mapView->setDestinationStation(m_dstCombo->itemData(index).toInt());
        updateRoute();
    }
}

void NavigatorPage::onSwapClicked() {
    int s = m_srcCombo->currentIndex(), d = m_dstCombo->currentIndex();
    m_srcCombo->setCurrentIndex(d);
    m_dstCombo->setCurrentIndex(s);
}

void NavigatorPage::onAlgSelected(bool isDijkstra) {
    m_useDijkstra = isDijkstra;
    m_btnDijkstra->setChecked(isDijkstra);
    m_btnBFS->setChecked(!isDijkstra);
    updateRoute();
}

void NavigatorPage::updateRoute() {
    if (!m_graph || !m_mapView) return;
    int src = m_mapView->getSourceStation();
    int dst = m_mapView->getDestinationStation();

    auto syncCombo = [](QComboBox* combo, int val) {
        int idx = combo->findData(val);
        if (idx != -1 && idx != combo->currentIndex()) {
            combo->blockSignals(true);
            combo->setCurrentIndex(idx);
            combo->blockSignals(false);
        }
    };
    syncCombo(m_srcCombo, src);
    syncCombo(m_dstCombo, dst);

    if (m_inspectedStationId == -1) m_inspectedStationId = src;

    if (src == -1 || dst == -1) {
        m_statsLabel->setText("<p style='color:#b3bdce;'>Choose a source and destination.</p>");
        m_timelineWidget->setRoute(RouteResult(), m_graph);
        m_mapView->clearRoute();
        showStationInfo(m_inspectedStationId);
        return;
    }

    RouteResult route = m_useDijkstra ?
        GraphAlgorithms::findFastestRouteDijkstra(*m_graph, src, dst) :
        GraphAlgorithms::findFewestStopsBFS(*m_graph, src, dst);

    if (!route.found) {
        m_statsLabel->setText("<p style='color:#e5484d; font-weight:bold;'>No route available between these stations.</p>");
        m_timelineWidget->setRoute(RouteResult(), m_graph);
        m_mapView->clearRoute();
        showStationInfo(m_inspectedStationId);
        return;
    }

    m_mapView->setActiveRoute(route, m_useDijkstra ? AppTheme::SuccessGreen : AppTheme::Cyan);

    m_statsLabel->setText(QString(
        "<table style='width:100%; border-collapse:separate; border-spacing:6px; margin:6px 0;'>"
        "<tr>"
        "<td style='border:1px solid #36435f; background-color:#1e2636; padding:6px 8px; width:33%;'><div style='font-size:24px; font-weight:bold; color:#fff;'>%1</div><div style='font-size:12px; color:#b3bdce;'>minutes</div></td>"
        "<td style='border:1px solid #36435f; background-color:#1e2636; padding:6px 8px; width:33%;'><div style='font-size:24px; font-weight:bold; color:#fff;'>%2</div><div style='font-size:12px; color:#b3bdce;'>stops</div></td>"
        "<td style='border:1px solid #36435f; background-color:#1e2636; padding:6px 8px; width:33%;'><div style='font-size:24px; font-weight:bold; color:#fff;'>%3</div><div style='font-size:12px; color:#b3bdce;'>interchanges</div></td>"
        "</tr></table>"
    ).arg(route.totalTime).arg(route.totalStops).arg(route.interchanges));

    m_timelineWidget->setRoute(route, m_graph);
    showStationInfo(m_inspectedStationId);
    emit routeUpdated();
}

void NavigatorPage::showStationInfo(int sid) {
    m_inspectedStationId = sid;
    if (!m_graph) { m_stationInfoLabel->clear(); return; }
    const Station* s = m_graph->getStation(sid);
    if (!s) { m_stationInfoLabel->clear(); return; }

    QString badges;
    for (const auto& l : s->lines) badges += AppTheme::lineBadgeHtml(l);
    if (badges.isEmpty()) badges = "none";
    bool closed = m_graph->isStationFailed(sid);

    m_stationInfoLabel->setText(QString(
        "<div style='margin-top:16px; margin-bottom:6px; font-size:12px; font-weight:bold; color:#b3bdce;'>STATION</div>"
        "<table style='width:100%; border-collapse:collapse; font-size:14px;'>"
        "<tr style='border-bottom:1px solid #36435f;'><td style='padding:6px 0; color:#b3bdce;'>Name</td><td style='padding:6px 0; text-align:right; color:#fff;'>%1</td></tr>"
        "<tr style='border-bottom:1px solid #36435f;'><td style='padding:6px 0; color:#b3bdce;'>Lines</td><td style='padding:6px 0; text-align:right;'>%2</td></tr>"
        "<tr style='border-bottom:1px solid #36435f;'><td style='padding:6px 0; color:#b3bdce;'>Interchange</td><td style='padding:6px 0; text-align:right; color:#fff;'>%3</td></tr>"
        "<tr style='border-bottom:1px solid #36435f;'><td style='padding:6px 0; color:#b3bdce;'>Status</td><td style='padding:6px 0; text-align:right; font-weight:bold; color:%4;'>%5</td></tr>"
        "</table>"
    ).arg(QString::fromStdString(s->name)).arg(badges).arg(s->lines.size() > 1 ? "Yes" : "No")
     .arg(closed ? AppTheme::ErrorRed : AppTheme::SuccessGreen).arg(closed ? "Closed" : "Open"));
}
