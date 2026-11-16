#ifndef APPTHEME_H
#define APPTHEME_H

#include <QString>
#include <QColor>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <vector>
#include <string>

namespace AppTheme {
    inline const QString BgDark       = "#151b28";
    inline const QString PanelDark    = "#1e2636";
    inline const QString BorderDark   = "#36435f";
    inline const QString TextLight    = "#eef1f6";
    inline const QString TextMuted    = "#b3bdce";
    inline const QString AccentBlue   = "#4a90e2";
    inline const QString SuccessGreen = "#2ecc71";
    inline const QString ErrorRed     = "#e5484d";
    inline const QString WarningAmber = "#f5a524";
    inline const QString Cyan         = "#25d0e8";
    inline const QString Purple       = "#b388ff";

    inline const QString LineBlue   = "#4c8dff";
    inline const QString LineYellow = "#e6c229";
    inline const QString LineOrange = "#f08a3c";
    inline const QString LineGreen  = "#3fb871";
    inline const QString LinePink   = "#d46fc9";

    inline QColor getLineColor(const std::string& name) {
        if (name == "Blue")   return QColor(LineBlue);
        if (name == "Yellow") return QColor(LineYellow);
        if (name == "Orange") return QColor(LineOrange);
        if (name == "Green")  return QColor(LineGreen);
        if (name == "Pink")   return QColor(LinePink);
        return QColor(TextMuted);
    }

    inline QString lineBadgeHtml(const std::string& name) {
        QString hex = QString::fromStdString(name);
        if (hex == "Blue") hex = LineBlue;
        else if (hex == "Yellow") hex = LineYellow;
        else if (hex == "Orange") hex = LineOrange;
        else if (hex == "Green") hex = LineGreen;
        else if (hex == "Pink") hex = LinePink;
        else hex = "#9aa5b8";

        return QString("<span style='background:%1; color:#111; padding:2px 7px; border-radius:3px; font-weight:600; font-size:11px; margin-right:4px;'>%2</span>")
               .arg(hex).arg(QString::fromStdString(name));
    }

    inline void styleTable(QTableWidget* table) {
        table->verticalHeader()->setVisible(false);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    inline void populateStationCombo(QComboBox* combo, const std::vector<std::pair<std::string, int>>& list) {
        int prev = combo->currentData().isValid() ? combo->currentData().toInt() : -1;
        combo->blockSignals(true);
        combo->clear();
        for (const auto& item : list) combo->addItem(QString::fromStdString(item.first), item.second);
        if (prev != -1) {
            int idx = combo->findData(prev);
            if (idx != -1) combo->setCurrentIndex(idx);
        }
        combo->blockSignals(false);
    }

    inline QString getStyleSheet() {
        return R"(
            QWidget { background-color: #1e2636; color: #eef1f6; font-family: 'Segoe UI', Arial; font-size: 13px; }
            #sidebar { background-color: #151b28; border-right: 1px solid #36435f; }
            #appTitle { font-size: 16px; font-weight: bold; color: #eef1f6; padding: 18px 14px; border-bottom: 1px solid #36435f; background: #151b28; }
            #navButton { text-align: left; padding: 12px 16px; border: none; border-left: 4px solid transparent; background: transparent; color: #b3bdce; font-size: 13px; font-weight: 500; }
            #navButton:hover { color: #ffffff; background-color: rgba(120, 140, 180, 0.1); }
            #navButton.active { color: #ffffff; border-left-color: #4a90e2; background-color: rgba(74, 144, 226, 0.15); font-weight: bold; }
            #hintBar { background-color: #1e2636; border-top: 1px solid #36435f; color: #b3bdce; padding: 8px 16px; font-size: 12px; }
            #panelScroll, #rightPanel { background-color: #1e2636; border: none; }
            #noteBox { background-color: #151b28; border: 1px solid #36435f; padding: 8px 10px; border-radius: 4px; color: #eef1f6; min-height: 36px; }
            QPushButton { background-color: #151b28; border: 1px solid #36435f; border-radius: 4px; padding: 6px 10px; color: #eef1f6; }
            QPushButton:hover { border-color: #4a90e2; }
            QPushButton:checked { background-color: #4a90e2; border-color: #4a90e2; color: #ffffff; }
            #segBtnLeft { border-top-right-radius: 0; border-bottom-right-radius: 0; margin-right: -1px; }
            #segBtnRight { border-top-left-radius: 0; border-bottom-left-radius: 0; }
            #primaryButton { background-color: #4a90e2; border-color: #4a90e2; color: #ffffff; font-weight: bold; }
            #primaryButton:hover { background-color: #3b7ac4; }
            QComboBox, QSpinBox, QLineEdit { background-color: #151b28; border: 1px solid #36435f; border-radius: 4px; padding: 5px 8px; color: #eef1f6; }
            QTableWidget { background-color: #151b28; border: 1px solid #36435f; border-radius: 4px; gridline-color: #242e42; color: #eef1f6; selection-background-color: rgba(74, 144, 226, 0.3); }
            QTableWidget::item { padding: 4px 6px; border-bottom: 1px solid #242e42; }
            QTableWidget QPushButton { background-color: #1a2233; border: 1px solid #36435f; border-radius: 4px; padding: 3px 8px; font-size: 11px; font-weight: 500; color: #eef1f6; min-height: 26px; }
            QTableWidget QPushButton:hover { border-color: #e5484d; color: #e5484d; background-color: #242c3d; }
            QHeaderView::section { background-color: #1a2233; color: #b3bdce; padding: 6px; border: none; border-bottom: 1px solid #36435f; font-weight: bold; font-size: 12px; }
            QScrollBar:vertical { background: #151b28; width: 8px; }
            QScrollBar::handle:vertical { background: #36435f; border-radius: 4px; }
        )";
    }
}

#endif // APPTHEME_H
