QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AltokepeRecepcionista
TEMPLATE = app

SOURCES += main.cpp \
           ../Ranking/Ranking.cpp \
           MainWindow.cpp \
           rankingwindow.cpp

HEADERS += MainWindow.h \
    ../Ranking/Ranking.h \
    rankingwindow.h