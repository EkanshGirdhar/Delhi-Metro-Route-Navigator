QT += core gui widgets

CONFIG += c++17

TARGET = MetroRouteSystem
TEMPLATE = app

SOURCES += \
    main.cpp \
    MetroGraph.cpp \
    GraphAlgorithms.cpp \
    VisualizerEngine.cpp \
    ResilienceEngine.cpp \
    MetroMapView.cpp \
    NavigatorPage.cpp \
    VisualizerPage.cpp \
    ResiliencePage.cpp \
    ManagementPage.cpp \
    MainWindow.cpp

HEADERS += \
    AppTheme.h \
    MetroGraph.h \
    GraphAlgorithms.h \
    VisualizerEngine.h \
    ResilienceEngine.h \
    MetroMapView.h \
    NavigatorPage.h \
    VisualizerPage.h \
    ResiliencePage.h \
    ManagementPage.h \
    MainWindow.h
