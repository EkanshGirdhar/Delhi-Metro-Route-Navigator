#ifndef VISUALIZERPAGE_H
#define VISUALIZERPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>
#include "MetroGraph.h"
#include "MetroMapView.h"
#include "VisualizerEngine.h"

class VisualizerPage : public QWidget {
    Q_OBJECT

public:
    explicit VisualizerPage(MetroGraph* graph, MetroMapView* mapView, QWidget* parent = nullptr);

    void populateStationCombos();
    void resetSimulation();
    void pauseSimulation();

private slots:
    void onAlgChanged(const QString& alg);
    void onPlayPauseClicked();
    void onNextClicked();
    void onEndClicked();
    void onResetClicked();
    void onTimerTick();

private:
    void setupUI();
    void updateVisuals();
    void updateComparisonTable();

    MetroGraph* m_graph = nullptr;
    MetroMapView* m_mapView = nullptr;

    QComboBox* m_srcCombo = nullptr;
    QComboBox* m_dstCombo = nullptr;
    QPushButton* m_btnDijkstra = nullptr;
    QPushButton* m_btnBFS = nullptr;
    QPushButton* m_btnDFS = nullptr;
    QPushButton* m_btnPlay = nullptr;
    QPushButton* m_btnNext = nullptr;
    QPushButton* m_btnEnd = nullptr;
    QPushButton* m_btnReset = nullptr;

    QLabel* m_stepProgressLabel = nullptr;
    QLabel* m_noteLabel = nullptr;
    QTableWidget* m_compareTable = nullptr;
    QLabel* m_distTableTitle = nullptr;
    QTableWidget* m_distTable = nullptr;
    QLabel* m_queueLabel = nullptr;
    QLabel* m_visitedLabel = nullptr;

    QString m_currentAlg = "d";
    std::vector<VisualizerStep> m_steps;
    int m_stepIndex = 0;
    QTimer* m_timer = nullptr;
};

#endif // VISUALIZERPAGE_H
