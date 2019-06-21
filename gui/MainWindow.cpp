#include "MainWindow.h"

MainWindow::MainWindow() {
    setWindowTitle("PopOut");
    //  setStyleSheet("QMainWindow::separator { width: 1; background-color: black}");

    board = new BoardWidget;
    searchWidget = new SearchWidget(board);
    setCentralWidget(board);

    QDockWidget *dock = new QDockWidget(tr("Search"), this);
    dock->setWidget(searchWidget);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    createActions();
    createMenus();
    board->setFocus();
    //mainWindow.resize(BOARD_WIDTH * 100, BOARD_HEIGHT * 100);
}

MainWindow::~MainWindow() {
    delete board;
}

void MainWindow::createActions() {
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    restartAct = new QAction(tr("&Restart"), this);
    restartAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    connect(restartAct, SIGNAL(triggered()), board, SLOT(restart()));

    variationAct = new QAction(tr("&Variation..."), this);
    variationAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    connect(variationAct, SIGNAL(triggered()), this, SLOT(changeVariation()));

    columnsAct = new QAction(tr("&Column mode"), this);
    //columnsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    columnsAct->setCheckable(true);
    connect(columnsAct, SIGNAL(triggered(bool)), searchWidget, SLOT(setColumnMode(bool)));
    
    handicapLimitAct = new QAction(tr("Handicap limits..."), this);
    connect(handicapLimitAct, SIGNAL(triggered()), searchWidget, SLOT(changeDepthLimit()));

    computerModeAct = new QAction(tr("Computer mode"), this);
    computerModeAct->setCheckable(true);
    connect(computerModeAct, SIGNAL(triggered(bool)), searchWidget, SLOT(setComputerMode(bool)));

    alphaBetaAct = new QAction(tr("Alpha-beta"), this);
    alphaBetaAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    connect(alphaBetaAct, SIGNAL(triggered()), searchWidget, SLOT(doAlphaBeta()));

    logicalAct = new QAction(tr("Handicap"), this);
    logicalAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    connect(logicalAct, SIGNAL(triggered()), searchWidget, SLOT(doLogical()));

    fullWidthAct = new QAction(tr("&Full-width search"), this);
    fullWidthAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    connect(fullWidthAct, SIGNAL(triggered()), searchWidget, SLOT(doFullWidth()));

    retroAct = new QAction(tr("Retrograde &analysis"), this);
    retroAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    connect(retroAct, SIGNAL(triggered()), searchWidget, SLOT(doRetrograde()));

    proofAct = new QAction(tr("&Proof-number"), this);
    proofAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connect(proofAct, SIGNAL(triggered()), searchWidget, SLOT(doProofNumber()));

    exactProofAct = new QAction(tr("PNS &exact"), this);
    exactProofAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(exactProofAct, SIGNAL(triggered()), searchWidget, SLOT(doExactProofNumber()));

    connect(searchWidget, SIGNAL(searchStarted()), this, SLOT(disableActions()));
    connect(searchWidget, SIGNAL(searchEnded()), this, SLOT(enableActions()));
    connect(searchWidget, SIGNAL(computerMoveFailed()), this, SLOT(disableComputerMode()));
}

void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);

    gameMenu = menuBar()->addMenu(tr("&Game"));
    gameMenu->addAction(restartAct);
    gameMenu->addAction(computerModeAct);
    gameMenu->addAction(variationAct);
    gameMenu->addSeparator();
    gameMenu->addAction(alphaBetaAct);
    gameMenu->addAction(logicalAct);
    gameMenu->addAction(retroAct);
    //gameMenu->addAction(fullWidthAct);
    gameMenu->addAction(proofAct);
    gameMenu->addAction(exactProofAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(columnsAct);
    settingsMenu->addAction(handicapLimitAct);
    QMenu* transMenu = settingsMenu->addMenu("TransTable size");
    QActionGroup *group = new QActionGroup(this);
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), searchWidget, SLOT(changeTransSize(int)));
    transMenu->addAction(createTransAction(group, mapper, 0));
    transMenu->addAction(createTransAction(group, mapper, 8388593));
    transMenu->addAction(createTransAction(group, mapper, 15999961));
    transMenu->addAction(createTransAction(group, mapper, 33554393));
    transMenu->addAction(createTransAction(group, mapper, 67108859));
    //transMenu->addAction(createTransAction(group, mapper, 100663243));
    transMenu->addAction(createTransAction(group, mapper, 134217689));
    transMenu->addAction(createTransAction(group, mapper, 268435399));
}

QAction* MainWindow::createTransAction(QActionGroup* group, QSignalMapper* mapper, int size) {
    QString title = "~";
    const int GIGA = 1024 * 1024 * 1024;
    const int MEGA = 1024 * 1024;

    qint64 bytes = (qint64) size * 16;

    if (bytes == 0) {
        title = "Disabled";
    } else if (bytes >= GIGA) {
        QString str = QString::number((double) bytes / GIGA, 'f', 2);
        title.append(str).append(" GB");
    } else if (bytes >= MEGA) {
        QString str = QString::number((double) bytes / MEGA, 'f', 0);
        title.append(str).append(" MB");
    } else {
        QString str = QString::number((double) bytes / 1024, 'f', 0);
        title.append(str).append(" kB");
    }

    QAction* act = new QAction(title, this);
    act->setCheckable(true);
    act->setChecked(size == SearchWorker::DEFAULT_TT_SIZE);
    act->setActionGroup(group);
    mapper->setMapping(act, size);
    connect(act, SIGNAL(triggered()), mapper, SLOT(map()));
    return act;
}

void MainWindow::changeVariation() {
    bool ok;
    QString var = QInputDialog::getText(this, tr("Variation"), tr("Enter variation:"), QLineEdit::Normal, board->getVariation(), &ok);
    if (ok)
        board->setVariation(var);
}

void MainWindow::enableActions() {
    gameMenu->setEnabled(true);
    settingsMenu->setEnabled(true);

}

void MainWindow::disableActions() {
    gameMenu->setEnabled(false);
    settingsMenu->setEnabled(false);
}

void MainWindow::disableComputerMode() {
    computerModeAct->setChecked(false);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
