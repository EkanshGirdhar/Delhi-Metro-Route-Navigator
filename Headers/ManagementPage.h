#ifndef MANAGEMENTPAGE_H
#define MANAGEMENTPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include "MetroGraph.h"
#include "MetroMapView.h"

class ManagementPage : public QWidget {
    Q_OBJECT

public:
    explicit ManagementPage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent = nullptr);
    void populateStationCombos();
    void updateManagement();
    void handleMapStationClick(int stationId);

signals:
    void networkModified();

private slots:
    void onAddConnectionClicked();
    void onClearLinkSelection();
    void onDeleteEdge(int edgeId);
    void onDeleteStation(int stationId);
    void onSearchStations(const QString& query);

private:
    void setupUI();

    MetroGraph* m_graph = nullptr;
    MetroMapView* m_mapView = nullptr;

    QComboBox* m_linkACombo = nullptr;
    QComboBox* m_linkBCombo = nullptr;
    QSpinBox* m_weightSpin = nullptr;
    QComboBox* m_lineCombo = nullptr;
    QPushButton* m_btnAdd = nullptr;
    QLabel* m_statusLabel = nullptr;

    QTableWidget* m_edgesTable = nullptr;
    QLineEdit* m_searchInput = nullptr;
    QTableWidget* m_stationsTable = nullptr;
};

#endif // MANAGEMENTPAGE_H
