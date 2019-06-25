QT += widgets
TEMPLATE = app
TARGET = popout
INCLUDEPATH += .

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += alphabeta.h \
           connect4.h \
           full.h \
           game.h \
           handicap.h \
           minimax.h \
           proof.h \
           retro.h \
           settings.h \
           transtable.h \
           gui/BoardWidget.h \
           gui/MainWindow.h \
           gui/SearchWidget.h \
           gui/SearchWorker.h
SOURCES += alphabeta.cpp \
           connect4.cpp \
           full.cpp \
           game.cpp \
           handicap.cpp \
           minimax.cpp \
           proof.cpp \
           retro.cpp \
           transtable.cpp \
           gui/BoardWidget.cpp \
           gui/MainWindow.cpp \
           gui/SearchWidget.cpp \
           gui/SearchWorker.cpp
