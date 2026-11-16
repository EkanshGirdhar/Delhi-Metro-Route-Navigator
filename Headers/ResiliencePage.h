#ifndef RESILIENCEPAGE_H
#define RESILIENCEPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QComboBox>
#include "MetroGraph.h"
#include "MetroMapView.h"

class ResiliencePage : public QWidget {
    Q_OBJECT

public:
    explicit ResiliencePage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent = nullptr);

    void populateStationCombos();
    void updateResilience();

private slots:
    void onToggleCriticalClicked();
    void onRestoreNetworkClicked();
    void onSrcChanged(int index);
    void onDstChanged(int index);
    void onAlgSelected(bool isDijkstra);

private:
    void setupUI();

    MetroGraph* m_graph = nullptr;
    MetroMapView* m_mapView = nullptr;
    bool m_showCritical = false;
    bool m_useDijkstra = true;

    QPushButton* m_btnToggleCritical = nullptr;
    QPushButton* m_btnRestoreNetwork = nullptr;
    QLabel* m_impactLabel = nullptr;
    QTableWidget* m_failuresTable = nullptr;
    QComboBox* m_srcCombo = nullptr;
    QComboBox* m_dstCombo = nullptr;
    QPushButton* m_btnDijkstra = nullptr;
    QPushButton* m_btnBFS = nullptr;
    QTableWidget* m_compareTable = nullptr;
    QLabel* m_routeLabel = nullptr;
};

#endif // RESILIENCEPAGE_H
