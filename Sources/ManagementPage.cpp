#include "ManagementPage.h"
#include "AppTheme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

using namespace std;

ManagementPage::ManagementPage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent)
    : QWidget(parent), m_graph(graph), m_mapView(mapView) {
    setupUI();
}

void ManagementPage::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    layout->addWidget(new QLabel("<h2>Station Management</h2>", this));

    QLabel* infoLabel = new QLabel("Manage tracks and stations in the active metro network.", this);
    infoLabel->setStyleSheet("color: #b3bdce; font-size: 11px;");
    layout->addWidget(infoLabel);

    layout->addWidget(new QLabel("<b>Add Track Connection</b>", this));

    auto addField = [this, layout](const QString& title, QWidget* widget) {
        layout->addWidget(new QLabel(title, this));
        layout->addWidget(widget);
    };

    m_linkACombo = new QComboBox(this);
    addField("From Station (A):", m_linkACombo);

    m_linkBCombo = new QComboBox(this);
    addField("To Station (B):", m_linkBCombo);

    m_weightSpin = new QSpinBox(this);
    m_weightSpin->setRange(1, 120);
    m_weightSpin->setValue(5);
    addField("Travel Time (minutes):", m_weightSpin);

    m_lineCombo = new QComboBox(this);
    m_lineCombo->addItems({"Blue", "Yellow", "Orange", "Green", "Pink"});
    addField("Metro Line:", m_lineCombo);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_btnAdd = new QPushButton("Add Connection", this);
    m_btnAdd->setObjectName("primaryButton");
    QPushButton* clearBtn = new QPushButton("Clear Selection", this);
    btnLayout->addWidget(m_btnAdd);
    btnLayout->addWidget(clearBtn);
    layout->addLayout(btnLayout);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    layout->addWidget(new QLabel("<b>Existing Connections</b>", this));
    m_edgesTable = new QTableWidget(0, 3, this);
    m_edgesTable->setHorizontalHeaderLabels({"Track", "Time", "Action"});
    m_edgesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_edgesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_edgesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_edgesTable->setColumnWidth(2, 95);
    AppTheme::styleTable(m_edgesTable);
    layout->addWidget(m_edgesTable);

    layout->addWidget(new QLabel("<b>Stations Directory</b>", this));
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Search stations by name...");
    layout->addWidget(m_searchInput);

    m_stationsTable = new QTableWidget(0, 3, this);
    m_stationsTable->setHorizontalHeaderLabels({"Name", "Lines", "Action"});
    m_stationsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_stationsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_stationsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_stationsTable->setColumnWidth(2, 95);
    AppTheme::styleTable(m_stationsTable);
    layout->addWidget(m_stationsTable);
    layout->addStretch();

    auto updateLink = [this](int) {
        if (m_mapView) {
            m_mapView->setManagementLink(m_linkACombo->currentData().toInt(), m_linkBCombo->currentData().toInt());
        }
    };

    connect(m_linkACombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, updateLink);
    connect(m_linkBCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, updateLink);
    connect(m_btnAdd, &QPushButton::clicked, this, &ManagementPage::onAddConnectionClicked);
    connect(clearBtn, &QPushButton::clicked, this, &ManagementPage::onClearLinkSelection);
    connect(m_searchInput, &QLineEdit::textChanged, this, &ManagementPage::onSearchStations);
}

void ManagementPage::populateStationCombos() {
    if (!m_graph) return;
    auto sorted = m_graph->getSortedStations();
    AppTheme::populateStationCombo(m_linkACombo, sorted);
    AppTheme::populateStationCombo(m_linkBCombo, sorted);
}

void ManagementPage::handleMapStationClick(int sid) {
    int a = m_linkACombo->currentData().toInt();
    int b = m_linkBCombo->currentData().toInt();

    if (a != -1 && b == -1 && a != sid) {
        m_linkBCombo->setCurrentIndex(m_linkBCombo->findData(sid));
    } else if (a != sid) {
        m_linkACombo->setCurrentIndex(m_linkACombo->findData(sid));
        m_linkBCombo->setCurrentIndex(-1);
    }

    if (m_mapView) {
        m_mapView->setManagementLink(m_linkACombo->currentData().toInt(), m_linkBCombo->currentData().toInt());
    }
}

void ManagementPage::onAddConnectionClicked() {
    if (!m_graph) return;
    int a = m_linkACombo->currentData().toInt();
    int b = m_linkBCombo->currentData().toInt();
    int weight = m_weightSpin->value();
    std::string line = m_lineCombo->currentText().toStdString();

    if (a == -1 || b == -1) {
        m_statusLabel->setText("<span style='color:#e5484d;'>Choose both stations.</span>");
        return;
    }
    if (a == b) {
        m_statusLabel->setText("<span style='color:#e5484d;'>Choose two different stations.</span>");
        return;
    }
    if (m_graph->areLinked(a, b)) {
        m_statusLabel->setText("<span style='color:#e5484d;'>Already connected.</span>");
        return;
    }

    int id = m_graph->getNextEdgeId();
    m_graph->addEdge(id, a, b, weight, line);

    m_statusLabel->setText("<span style='color:#2ecc71;'>Track connection added successfully.</span>");
    onClearLinkSelection();
    updateManagement();
    emit networkModified();
}

void ManagementPage::onClearLinkSelection() {
    m_linkACombo->setCurrentIndex(-1);
    m_linkBCombo->setCurrentIndex(-1);
    if (m_mapView) {
        m_mapView->setManagementLink(-1, -1);
    }
}

void ManagementPage::onDeleteEdge(int edgeId) {
    if (!m_graph) return;
    m_graph->removeEdge(edgeId);
    updateManagement();
    emit networkModified();
}

void ManagementPage::onDeleteStation(int stationId) {
    if (!m_graph) return;
    m_graph->removeStation(stationId);
    populateStationCombos();
    updateManagement();
    emit networkModified();
}

void ManagementPage::onSearchStations(const QString& query) {
    if (!m_graph) return;
    QString q = query.trimmed().toLower();
    std::vector<const Station*> matches;

    for (const auto& pair : m_graph->getStations()) {
        if (q.isEmpty() || QString::fromStdString(pair.second.name).toLower().contains(q)) {
            matches.push_back(&pair.second);
        }
    }

    m_stationsTable->setRowCount(static_cast<int>(matches.size()));
    for (size_t row = 0; row < matches.size(); ++row) {
        const Station* s = matches[row];
        m_stationsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(s->name)));

        QString lines;
        for (const auto& lineName : s->lines) {
            lines += QString::fromStdString(lineName) + " ";
        }
        m_stationsTable->setItem(row, 1, new QTableWidgetItem(lines.trimmed()));

        QPushButton* btn = new QPushButton("Delete");
        int sid = s->id;
        connect(btn, &QPushButton::clicked, this, [this, sid]() { onDeleteStation(sid); });
        m_stationsTable->setCellWidget(row, 2, btn);
        m_stationsTable->setRowHeight(row, 36);
    }

    int headerH = std::max(26, m_stationsTable->horizontalHeader()->height());
    m_stationsTable->setFixedHeight(headerH + static_cast<int>(matches.size()) * 36 + m_stationsTable->frameWidth() * 2 + 4);
}

void ManagementPage::updateManagement() {
    if (!m_graph) return;

    const auto& edges = m_graph->getEdges();
    m_edgesTable->setRowCount(static_cast<int>(edges.size()));
    int row = 0;

    for (const auto& pair : edges) {
        const Edge& e = pair.second;
        const Station* sa = m_graph->getStation(e.u);
        const Station* sb = m_graph->getStation(e.v);
        QString aName = sa ? QString::fromStdString(sa->name) : "?";
        QString bName = sb ? QString::fromStdString(sb->name) : "?";

        m_edgesTable->setItem(row, 0, new QTableWidgetItem(aName + " - " + bName + " (" + QString::fromStdString(e.line) + ")"));
        m_edgesTable->setItem(row, 1, new QTableWidgetItem(QString::number(e.weight) + " m"));

        QPushButton* btn = new QPushButton("Remove");
        int eid = e.id;
        connect(btn, &QPushButton::clicked, this, [this, eid]() { onDeleteEdge(eid); });
        m_edgesTable->setCellWidget(row, 2, btn);
        m_edgesTable->setRowHeight(row, 36);
        row++;
    }

    int headerH = std::max(26, m_edgesTable->horizontalHeader()->height());
    m_edgesTable->setFixedHeight(headerH + static_cast<int>(edges.size()) * 36 + m_edgesTable->frameWidth() * 2 + 4);

    onSearchStations(m_searchInput->text());
}
