#include "VisualizerPage.h"
#include "GraphAlgorithms.h"
#include "AppTheme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <climits>

using namespace std;

VisualizerPage::VisualizerPage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent)
    : QWidget(parent), m_graph(graph), m_mapView(mapView), m_timer(new QTimer(this)) {
    setupUI();
    connect(m_timer, &QTimer::timeout, this, &VisualizerPage::onTimerTick);
}

void VisualizerPage::setupUI() {
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setContentsMargins(16, 16, 16, 16);
    l->setSpacing(8);

    l->addWidget(new QLabel("<h2>Algorithm Visualizer</h2>", this));
    l->addWidget(new QLabel("Source:", this));
    m_srcCombo = new QComboBox(this); l->addWidget(m_srcCombo);
    l->addWidget(new QLabel("Destination:", this));
    m_dstCombo = new QComboBox(this); l->addWidget(m_dstCombo);

    QHBoxLayout* algChoice = new QHBoxLayout();
    m_btnDijkstra = new QPushButton("Dijkstra", this);
    m_btnBFS = new QPushButton("BFS", this);
    m_btnDFS = new QPushButton("DFS", this);
    m_btnDijkstra->setCheckable(true); m_btnBFS->setCheckable(true); m_btnDFS->setCheckable(true);
    m_btnDijkstra->setChecked(true);
    algChoice->addWidget(m_btnDijkstra); algChoice->addWidget(m_btnBFS); algChoice->addWidget(m_btnDFS);
    l->addLayout(algChoice);

    QHBoxLayout* playback = new QHBoxLayout();
    m_btnPlay = new QPushButton("Play", this); m_btnPlay->setObjectName("primaryButton");
    m_btnNext = new QPushButton("Next", this);
    m_btnEnd = new QPushButton("End", this);
    m_btnReset = new QPushButton("Reset", this);
    playback->addWidget(m_btnPlay); playback->addWidget(m_btnNext); playback->addWidget(m_btnEnd); playback->addWidget(m_btnReset);
    l->addLayout(playback);

    l->addWidget(new QLabel("<b>Algorithm Comparison</b>", this));
    m_compareTable = new QTableWidget(3, 4, this);
    m_compareTable->setHorizontalHeaderLabels({"Algorithm", "Min", "Stops", "Changes"});
    m_compareTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int c = 1; c < 4; ++c) m_compareTable->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
    AppTheme::styleTable(m_compareTable);
    m_compareTable->setFixedHeight(120);
    l->addWidget(m_compareTable);

    m_stepProgressLabel = new QLabel(this);
    m_stepProgressLabel->setStyleSheet("font-weight: bold; color: #4a90e2; font-size: 14px; margin-top: 6px;");
    l->addWidget(m_stepProgressLabel);

    m_noteLabel = new QLabel(this);
    m_noteLabel->setObjectName("noteBox");
    m_noteLabel->setWordWrap(true);
    l->addWidget(m_noteLabel);

    m_distTableTitle = new QLabel("<h3 style='margin-top:12px; margin-bottom:4px; font-size:13px; color:#b3bdce;'>TABLE</h3>", this);
    l->addWidget(m_distTableTitle);

    m_distTable = new QTableWidget(0, 3, this);
    m_distTable->setHorizontalHeaderLabels({"Station", "Distance", "Previous"});
    m_distTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_distTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_distTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    AppTheme::styleTable(m_distTable);
    l->addWidget(m_distTable);

    l->addWidget(new QLabel("Frontier / Queue:", this));
    m_queueLabel = new QLabel("Empty", this);
    m_queueLabel->setStyleSheet("background-color:#151b28; border:1px solid #36435f; padding:6px; border-radius:4px; font-size:11px;");
    m_queueLabel->setWordWrap(true);
    l->addWidget(m_queueLabel);

    l->addWidget(new QLabel("Visited Sequence:", this));
    m_visitedLabel = new QLabel("None", this);
    m_visitedLabel->setStyleSheet("background-color:#151b28; border:1px solid #36435f; padding:6px; border-radius:4px; color:#2ecc71; font-size:11px;");
    m_visitedLabel->setWordWrap(true);
    l->addWidget(m_visitedLabel);
    l->addStretch();

    connect(m_srcCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx >= 0 && m_mapView) { m_mapView->setSourceStation(m_srcCombo->itemData(idx).toInt()); resetSimulation(); }
    });
    connect(m_dstCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx >= 0 && m_mapView) { m_mapView->setDestinationStation(m_dstCombo->itemData(idx).toInt()); resetSimulation(); }
    });
    connect(m_btnDijkstra, &QPushButton::clicked, this, [this]() { onAlgChanged("d"); });
    connect(m_btnBFS,      &QPushButton::clicked, this, [this]() { onAlgChanged("b"); });
    connect(m_btnDFS,      &QPushButton::clicked, this, [this]() { onAlgChanged("f"); });
    connect(m_btnPlay,     &QPushButton::clicked, this, &VisualizerPage::onPlayPauseClicked);
    connect(m_btnNext,     &QPushButton::clicked, this, &VisualizerPage::onNextClicked);
    connect(m_btnEnd,      &QPushButton::clicked, this, &VisualizerPage::onEndClicked);
    connect(m_btnReset,    &QPushButton::clicked, this, &VisualizerPage::onResetClicked);
}

void VisualizerPage::populateStationCombos() {
    if (!m_graph) return;
    auto sorted = m_graph->getSortedStations();
    AppTheme::populateStationCombo(m_srcCombo, sorted);
    AppTheme::populateStationCombo(m_dstCombo, sorted);
    int i0 = m_srcCombo->findData(0), i4 = m_dstCombo->findData(4);
    if (i0 != -1) m_srcCombo->setCurrentIndex(i0);
    if (i4 != -1) m_dstCombo->setCurrentIndex(i4);
}

void VisualizerPage::resetSimulation() { onResetClicked(); }
void VisualizerPage::pauseSimulation() {
    m_timer->stop();
    m_btnPlay->setText("Play");
}

void VisualizerPage::onAlgChanged(const QString& alg) {
    m_currentAlg = alg;
    m_btnDijkstra->setChecked(alg == "d");
    m_btnBFS->setChecked(alg == "b");
    m_btnDFS->setChecked(alg == "f");
    onResetClicked();
}

void VisualizerPage::onResetClicked() {
    pauseSimulation();
    if (!m_graph || !m_mapView) return;

    int src = m_mapView->getSourceStation(), dst = m_mapView->getDestinationStation();
    auto sync = [](QComboBox* c, int v) {
        int i = c->findData(v);
        if (i != -1 && i != c->currentIndex()) { c->blockSignals(true); c->setCurrentIndex(i); c->blockSignals(false); }
    };
    sync(m_srcCombo, src); sync(m_dstCombo, dst);

    if (src == -1) {
        m_steps.clear(); m_stepIndex = 0;
        m_mapView->setVisualizerStep(nullptr);
        updateVisuals(); return;
    }

    if (m_currentAlg == "d") m_steps = VisualizerEngine::generateDijkstraSteps(*m_graph, src, dst);
    else if (m_currentAlg == "b") m_steps = VisualizerEngine::generateBFSSteps(*m_graph, src, dst);
    else m_steps = VisualizerEngine::generateDFSSteps(*m_graph, src, dst);

    m_stepIndex = 0;
    updateComparisonTable();
    updateVisuals();
}

void VisualizerPage::onPlayPauseClicked() {
    if (m_steps.empty()) return;
    if (m_timer->isActive()) pauseSimulation();
    else {
        if (m_stepIndex >= static_cast<int>(m_steps.size()) - 1) m_stepIndex = 0;
        m_timer->start(500);
        m_btnPlay->setText("Pause");
    }
}

void VisualizerPage::onNextClicked() {
    pauseSimulation();
    if (!m_steps.empty() && m_stepIndex < static_cast<int>(m_steps.size()) - 1) { m_stepIndex++; updateVisuals(); }
}

void VisualizerPage::onEndClicked() {
    pauseSimulation();
    if (!m_steps.empty()) { m_stepIndex = static_cast<int>(m_steps.size()) - 1; updateVisuals(); }
}

void VisualizerPage::onTimerTick() {
    if (m_stepIndex < static_cast<int>(m_steps.size()) - 1) { m_stepIndex++; updateVisuals(); }
    else pauseSimulation();
}

void VisualizerPage::updateComparisonTable() {
    if (!m_graph || !m_mapView) return;
    int src = m_mapView->getSourceStation(), dst = m_mapView->getDestinationStation();
    if (src == -1 || dst == -1) return;

    const struct { QString name, key; RouteResult res; } rows[] = {
        {"Dijkstra", "d", GraphAlgorithms::findFastestRouteDijkstra(*m_graph, src, dst)},
        {"BFS", "b", GraphAlgorithms::findFewestStopsBFS(*m_graph, src, dst)},
        {"DFS", "f", GraphAlgorithms::findRouteDFS(*m_graph, src, dst)}
    };

    for (int r = 0; r < 3; ++r) {
        m_compareTable->setItem(r, 0, new QTableWidgetItem(rows[r].name));
        if (rows[r].res.found) {
            m_compareTable->setItem(r, 1, new QTableWidgetItem(QString::number(rows[r].res.totalTime)));
            m_compareTable->setItem(r, 2, new QTableWidgetItem(QString::number(rows[r].res.totalStops)));
            m_compareTable->setItem(r, 3, new QTableWidgetItem(QString::number(rows[r].res.interchanges)));
        } else {
            m_compareTable->setItem(r, 1, new QTableWidgetItem("None"));
            m_compareTable->setItem(r, 2, new QTableWidgetItem("-"));
            m_compareTable->setItem(r, 3, new QTableWidgetItem("-"));
        }
        bool sel = (m_currentAlg == rows[r].key);
        for (int c = 0; c < 4; ++c) {
            if (m_compareTable->item(r, c)) m_compareTable->item(r, c)->setBackground(sel ? QColor(74, 144, 226, 60) : Qt::transparent);
        }
        m_compareTable->setRowHeight(r, 28);
    }
    int th = m_compareTable->horizontalHeader()->height() + 3 * 28 + m_compareTable->frameWidth() * 2 + 4;
    m_compareTable->setFixedHeight(th);
}

void VisualizerPage::updateVisuals() {
    if (m_steps.empty() || m_stepIndex >= static_cast<int>(m_steps.size())) {
        m_stepProgressLabel->setText("Select Source and Destination to begin.");
        m_noteLabel->setText("No simulation active.");
        m_distTable->setRowCount(0); m_distTable->setFixedHeight(40);
        m_queueLabel->setText("Empty"); m_visitedLabel->setText("None");
        if (m_mapView) m_mapView->setVisualizerStep(nullptr);
        return;
    }

    const VisualizerStep& step = m_steps[m_stepIndex];
    if (m_mapView) m_mapView->setVisualizerStep(&step);

    m_stepProgressLabel->setText(QString("Step %1 of %2").arg(m_stepIndex + 1).arg(m_steps.size()));
    m_noteLabel->setText(QString::fromStdString(step.note));

    QString colName = (m_currentAlg == "d") ? "Distance" : (m_currentAlg == "b") ? "Level" : "Order";
    m_distTable->setHorizontalHeaderLabels({"Station", colName, "Previous"});

    std::vector<int> sOrder;
    for (const auto& p : step.distances) sOrder.push_back(p.first);
    std::sort(sOrder.begin(), sOrder.end());

    m_distTable->setRowCount(static_cast<int>(sOrder.size()));
    const QString inf = QString(QChar(0x221E));

    for (size_t r = 0; r < sOrder.size(); ++r) {
        int sid = sOrder[r];
        const Station* st = m_graph->getStation(sid);
        int d = step.distances.at(sid);
        QString prev = "-";
        auto it = step.previous.find(sid);
        if (it != step.previous.end()) {
            const Station* sp = m_graph->getStation(it->second);
            prev = sp ? QString::fromStdString(sp->name) : QString::number(it->second);
        }

        m_distTable->setItem(r, 0, new QTableWidgetItem(st ? QString::fromStdString(st->name) : QString::number(sid)));
        auto* itemD = new QTableWidgetItem(d == INT_MAX ? inf : QString::number(d));
        itemD->setTextAlignment(Qt::AlignCenter);
        m_distTable->setItem(r, 1, itemD);
        auto* itemP = new QTableWidgetItem(prev);
        itemP->setTextAlignment(Qt::AlignCenter);
        m_distTable->setItem(r, 2, itemP);
        m_distTable->setRowHeight(r, 28);

        if (sid == step.currentStation) {
            for (int c = 0; c < 3; ++c) m_distTable->item(r, c)->setBackground(QColor(37, 208, 232, 70));
        }
    }

    int hh = std::max(26, m_distTable->horizontalHeader()->height());
    m_distTable->setFixedHeight(hh + static_cast<int>(sOrder.size()) * 28 + m_distTable->frameWidth() * 2 + 4);

    QString qStr;
    for (size_t i = 0; i < step.frontierQueue.size(); ++i) qStr += (i > 0 ? ", " : "") + QString::fromStdString(step.frontierQueue[i]);
    m_queueLabel->setText(qStr.isEmpty() ? "Empty" : qStr);

    QString vStr;
    for (size_t i = 0; i < step.visitedOrder.size(); ++i) {
        const Station* sv = m_graph->getStation(step.visitedOrder[i]);
        vStr += (i > 0 ? " -> " : "") + (sv ? QString::fromStdString(sv->name) : QString::number(step.visitedOrder[i]));
    }
    m_visitedLabel->setText(vStr.isEmpty() ? "None" : vStr);

    int src = m_mapView ? m_mapView->getSourceStation() : -1;
    int dst = m_mapView ? m_mapView->getDestinationStation() : -1;
    if (dst != -1 && m_stepIndex == static_cast<int>(m_steps.size()) - 1 && m_mapView) {
        RouteResult r = m_currentAlg == "d" ? GraphAlgorithms::findFastestRouteDijkstra(*m_graph, src, dst) :
                        m_currentAlg == "b" ? GraphAlgorithms::findFewestStopsBFS(*m_graph, src, dst) :
                                              GraphAlgorithms::findRouteDFS(*m_graph, src, dst);
        m_mapView->setActiveRoute(r, m_currentAlg == "d" ? AppTheme::SuccessGreen : m_currentAlg == "b" ? AppTheme::Cyan : AppTheme::Purple);
    }
}
