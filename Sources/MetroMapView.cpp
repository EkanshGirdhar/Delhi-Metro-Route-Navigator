#include "MetroMapView.h"
#include "AppTheme.h"
#include "ResilienceEngine.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>

using namespace std;

MetroMapView::MetroMapView(QWidget* parent) : QWidget(parent), m_routeColor(QColor(AppTheme::SuccessGreen)) {
    setMouseTracking(true);
    setMinimumSize(600, 400);
}

void MetroMapView::setGraph(MetroGraph* g) { m_graph = g; update(); }
void MetroMapView::setViewMode(ViewMode m) { m_viewMode = m; update(); }
void MetroMapView::setSourceStation(int id) { m_srcStation = id; update(); }
void MetroMapView::setDestinationStation(int id) { m_dstStation = id; update(); }
void MetroMapView::setSelectedStation(int id) { m_selectedStation = id; update(); }
void MetroMapView::setManagementLink(int a, int b) { m_linkA = a; m_linkB = b; update(); }
void MetroMapView::setShowCritical(bool show) { m_showCritical = show; update(); }

void MetroMapView::setActiveRoute(const RouteResult& route, const QString& algColor) {
    m_routeEdges.clear(); m_routeNodes.clear();
    m_routeColor = QColor(algColor);
    if (route.found) {
        for (int eid : route.edgePath) m_routeEdges.insert(eid);
        for (int sid : route.stationPath) m_routeNodes.insert(sid);
    }
    update();
}

void MetroMapView::setVisualizerStep(const VisualizerStep* step) { m_currentStep = step; update(); }
void MetroMapView::clearRoute() { m_routeEdges.clear(); m_routeNodes.clear(); m_currentStep = nullptr; update(); }

double MetroMapView::scaleFactor() const {
    return std::min(static_cast<double>(width()) / MAP_WIDTH, static_cast<double>(height()) / MAP_HEIGHT);
}

QPointF MetroMapView::mapToScreen(const QPointF& pt) const {
    double s = scaleFactor();
    return QPointF((width() - MAP_WIDTH * s) / 2.0 + pt.x() * s, (height() - MAP_HEIGHT * s) / 2.0 + pt.y() * s);
}

QPointF MetroMapView::screenToMap(const QPointF& pt) const {
    double s = scaleFactor();
    if (s <= 0.0) return pt;
    return QPointF((pt.x() - (width() - MAP_WIDTH * s) / 2.0) / s, (pt.y() - (height() - MAP_HEIGHT * s) / 2.0) / s);
}

double MetroMapView::distanceToSegment(const QPointF& p, const QPointF& a, const QPointF& b) const {
    double dx = b.x() - a.x(), dy = b.y() - a.y();
    double lenSq = dx * dx + dy * dy;
    if (lenSq == 0.0) return std::hypot(p.x() - a.x(), p.y() - a.y());
    double t = std::max(0.0, std::min(1.0, ((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / lenSq));
    return std::hypot(p.x() - (a.x() + t * dx), p.y() - (a.y() + t * dy));
}

int MetroMapView::findStationAt(const QPointF& pt) const {
    if (!m_graph) return -1;
    for (const auto& p : m_graph->getStations()) {
        if (std::hypot(pt.x() - p.second.x, pt.y() - p.second.y) <= 24.0) return p.second.id;
    }
    return -1;
}

int MetroMapView::findEdgeAt(const QPointF& pt) const {
    if (!m_graph) return -1;
    int closest = -1;
    double minD = 16.0;
    for (const auto& p : m_graph->getEdges()) {
        const Station* sa = m_graph->getStation(p.second.u), *sb = m_graph->getStation(p.second.v);
        if (!sa || !sb) continue;
        double d = distanceToSegment(pt, QPointF(sa->x, sa->y), QPointF(sb->x, sb->y));
        if (d < minD) { minD = d; closest = p.second.id; }
    }
    return closest;
}

void MetroMapView::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        QPointF pt = screenToMap(e->position());
        int sid = findStationAt(pt);
        if (sid != -1) { emit stationClicked(sid); return; }
        int eid = findEdgeAt(pt);
        if (eid != -1) { emit edgeClicked(eid); return; }
    }
    QWidget::mousePressEvent(e);
}

void MetroMapView::mouseMoveEvent(QMouseEvent* e) {
    QPointF pt = screenToMap(e->position());
    setCursor((findStationAt(pt) != -1 || findEdgeAt(pt) != -1) ? Qt::PointingHandCursor : Qt::ArrowCursor);
    QWidget::mouseMoveEvent(e);
}

void MetroMapView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor(AppTheme::BgDark));
    if (!m_graph) return;

    ResilienceAnalysis res;
    if (m_viewMode == ViewResilience) res = ResilienceEngine::analyze(*m_graph);

    double s = scaleFactor();

    for (const auto& pair : m_graph->getEdges()) {
        const Edge& e = pair.second;
        const Station* sa = m_graph->getStation(e.u), *sb = m_graph->getStation(e.v);
        if (!sa || !sb) continue;
        QPointF pA = mapToScreen(QPointF(sa->x, sa->y)), pB = mapToScreen(QPointF(sb->x, sb->y));

        bool failed = m_graph->isEdgeFailed(e.id) || m_graph->isStationFailed(e.u) || m_graph->isStationFailed(e.v);
        QColor col = AppTheme::getLineColor(e.line);
        double w = 5.0 * s;
        Qt::PenStyle style = Qt::SolidLine;

        if (failed) { col = QColor(AppTheme::ErrorRed); style = Qt::DashLine; w = 4.0 * s; }
        else if (m_viewMode == ViewResilience && m_showCritical && res.bridgeEdges.count(e.id)) { col = QColor(AppTheme::WarningAmber); w = 8.0 * s; }
        else if (m_routeEdges.count(e.id)) { col = m_routeColor; w = 8.0 * s; }

        if (m_currentStep) {
            for (int actId : m_currentStep->activeEdges) {
                if (actId == e.id) { col = QColor("#ffffff"); w = 9.0 * s; break; }
            }
        }

        p.setPen(QPen(col, std::max(2.0, w), style, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(pA, pB);

        QPointF mid = (pA + pB) / 2.0;
        QString wStr = QString::number(e.weight);
        QFont f("Segoe UI", std::max(8, static_cast<int>(10 * s)), QFont::Bold);
        p.setFont(f);
        QRect r = QFontMetrics(f).boundingRect(wStr).adjusted(-4, -2, 4, 2);
        r.moveCenter(mid.toPoint() - QPoint(0, static_cast<int>(8 * s)));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(21, 27, 40, 200));
        p.drawRoundedRect(r, 3, 3);
        p.setPen(QColor(AppTheme::TextMuted));
        p.drawText(r, Qt::AlignCenter, wStr);
    }

    if (m_viewMode == ViewManagement && m_linkA != -1 && m_linkB != -1) {
        const Station* sa = m_graph->getStation(m_linkA);
        const Station* sb = m_graph->getStation(m_linkB);
        if (sa && sb) {
            p.setPen(QPen(QColor(AppTheme::AccentBlue), 4.0 * s, Qt::DashLine, Qt::RoundCap));
            p.drawLine(mapToScreen(QPointF(sa->x, sa->y)), mapToScreen(QPointF(sb->x, sb->y)));
        }
    }

    const QString groupPal[] = {"#9aa5b8", "#b388ff", "#4dd4a0", "#ff7eb6", "#6cb6ff"};
    for (const auto& pair : m_graph->getStations()) {
        const Station& st = pair.second;
        QPointF pt = mapToScreen(QPointF(st.x, st.y));
        bool isInter = st.lines.size() > 1, failed = m_graph->isStationFailed(st.id);
        double rad = (isInter ? 14.0 : 10.0) * s, stroke = (isInter ? 3.5 : 2.5) * s;
        QColor fill(AppTheme::BgDark), strokeCol(AppTheme::TextLight);

        if (m_routeNodes.count(st.id)) fill = m_routeColor;
        if (m_currentStep) {
            if (std::find(m_currentStep->visitedOrder.begin(), m_currentStep->visitedOrder.end(), st.id) != m_currentStep->visitedOrder.end()) {
                fill = QColor(AppTheme::LineGreen);
            }
            if (m_currentStep->currentStation == st.id) { fill = QColor(AppTheme::Cyan); rad += 3.0 * s; }
        }
        if (m_viewMode == ViewResilience && m_showCritical && res.articulationStations.count(st.id)) {
            strokeCol = QColor(AppTheme::WarningAmber); stroke = 5.0 * s;
        } else if (m_viewMode == ViewResilience && res.connectedComponents > 1 && !failed) {
            strokeCol = QColor(groupPal[res.componentMap[st.id] % 5]);
        }
        if (failed) { fill = QColor(AppTheme::ErrorRed); strokeCol = QColor(AppTheme::ErrorRed); }

        bool ring = (m_viewMode == ViewManagement) ? (st.id == m_linkA || st.id == m_linkB) : (st.id == m_selectedStation && m_viewMode != ViewResilience);
        if (ring) {
            p.setPen(QPen(QColor(AppTheme::AccentBlue), 2.0 * s, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(pt, rad + 6.0 * s, rad + 6.0 * s);
        }

        p.setPen(QPen(strokeCol, std::max(1.5, stroke)));
        p.setBrush(fill);
        p.drawEllipse(pt, rad, rad);

        QString mark = (m_viewMode == ViewManagement) ? (st.id == m_linkA ? "A" : st.id == m_linkB ? "B" : "") :
                                                         (st.id == m_srcStation ? "A" : st.id == m_dstStation ? "B" : "");
        if (!mark.isEmpty()) {
            QFont mf("Segoe UI", std::max(8, static_cast<int>(10 * s)), QFont::Bold);
            p.setFont(mf); p.setPen(QColor("#111111"));
            p.drawText(QRectF(pt.x() - rad, pt.y() - rad, rad * 2, rad * 2), Qt::AlignCenter, mark);
        }

        QFont lf("Segoe UI", std::max(9, static_cast<int>(11 * s)), QFont::DemiBold);
        p.setFont(lf);
        QString sName = QString::fromStdString(st.name);
        int tw = QFontMetrics(lf).horizontalAdvance(sName), th = QFontMetrics(lf).height();
        QPointF lPos(pt.x() + rad + 4 * s, pt.y() + th / 4.0);
        if (st.x > 800) lPos.setX(pt.x() - rad - tw - 4 * s);
        else if (st.y > 500) { lPos.setY(pt.y() + rad + th); lPos.setX(pt.x() - tw / 2.0); }
        else if (st.y < 150) { lPos.setY(pt.y() - rad - 4 * s); lPos.setX(pt.x() - tw / 2.0); }

        p.setPen(QColor(AppTheme::BgDark));
        for (int dx = -1; dx <= 1; ++dx) for (int dy = -1; dy <= 1; ++dy) if (dx || dy) p.drawText(lPos + QPointF(dx, dy), sName);
        p.setPen(QColor(AppTheme::TextLight));
        p.drawText(lPos, sName);
    }
}
