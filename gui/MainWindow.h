#ifndef MAINWINDOW_H
#define	MAINWINDOW_H

#include <QtWidgets>
#include "BoardWidget.h"
#include "SearchWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public slots:
    void changeVariation();
    void enableActions();
    void disableActions();
    void disableComputerMode();

private:
    void createMenus();
    void createActions();
    QAction* createTransAction(QActionGroup*, QSignalMapper*, int size);

    BoardWidget* board;
    SearchWidget* searchWidget;

    QMenu* gameMenu;
    QMenu *settingsMenu;
    QAction* variationAct;
    QAction* restartAct;
    QAction* columnsAct;
    QAction* handicapLimitAct;
    QAction* computerModeAct;
    QAction* alphaBetaAct;
    QAction* logicalAct;
    QAction* fullWidthAct;
    QAction* retroAct;
    QAction* proofAct;
    QAction* exactProofAct;
    QAction* exitAct;
};


#endif
