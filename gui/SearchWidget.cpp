#include <sstream>
#include <iostream>

#include "SearchWidget.h"
#include "minimax.h"
#include "handicap.h"

SearchWidget::SearchWidget(BoardWidget* bw) : boardWidget(bw) {
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setFontPointSize(11);
    textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* fontAction = new QAction("Big font", this);
    QAction* copyAction = new QAction("Copy all", this);
    // textEdit->addAction(fontAction);
    textEdit->addAction(copyAction);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(textEdit);
    layout->setContentsMargins(0, 0, 0, 0);

    workerThread = new QThread;
    worker = new SearchWorker;
    worker->moveToThread(workerThread);
    //start the event loop for worker thread
    workerThread->start();
    available = true;
    columnMode = false;
    computerMode = false;

    qRegisterMetaType<SearchWorker::SearchRequest>("SearchWorker::SearchRequest");
    connect(worker, &SearchWorker::resultsReady, this, &SearchWidget::setResults);
    connect(worker, &SearchWorker::update, this, &SearchWidget::setText);
    connect(worker, &SearchWorker::updateLamp, this, &SearchWidget::setLamp);
    connect(worker, &SearchWorker::computerMove, this, &SearchWidget::makeComputerMove);
    connect(this, &SearchWidget::columnSearchStarted, worker, &SearchWorker::processColumns);
    connect(this, &SearchWidget::singleSearchStarted, worker, &SearchWorker::processSingle);
    connect(this, &SearchWidget::computerSearchStarted, worker, &SearchWorker::processComputer);
    connect(fontAction, SIGNAL(triggered()), this, SLOT(changeFontSize()));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyAll()));
    connect(boardWidget, &BoardWidget::positionChanged, this, &SearchWidget::setVariation);
    connect(this, &SearchWidget::destroyed, workerThread, &QObject::deleteLater);
    connect(this, &SearchWidget::destroyed, worker, &QObject::deleteLater);
}

SearchWidget::~SearchWidget() {

}

void SearchWidget::changeFontSize() {
    textEdit->setFontPointSize(20);
}

void SearchWidget::copyAll() {
    QApplication::clipboard()->setText(textEdit->toPlainText());
}

bool SearchWidget::isLocked() {
    if (!available) {
        QMessageBox::information(this, tr("Search not finished"), tr("The previous search has not finished yet. Please wait"));
        return true;
    }
    return false;
}

void SearchWidget::lock() {
    emit searchStarted();
    available = false;
    boardWidget->allowMoves(false);
    boardWidget->clearLamps();
}

void SearchWidget::changeTransSize(int size) {
    if (isLocked()) {
        return;
    }
    try {
        worker->setTransTableSize(size);
    } catch (const std::bad_alloc&) {
        QMessageBox::critical(this, "Not enough memory", "Either you are out of memory or the program is running as 32-bit");
        exit(1);
    }
}

void SearchWidget::doAlphaBeta() {
    if (isLocked()) return;

    bool ok;
    int maxDepth = QInputDialog::getInt(this, tr("Enter search depth"), tr("Max depth:"), 1000, 1, 10000, 1, &ok);
    if (!ok) {
        return;
    }

    lock();
    SearchWorker::SearchRequest request;
    request.type = SearchWorker::AlphaBetaRequest;
    request.maxDepth = maxDepth;
    request.variation = boardWidget->getVariation();

    if (columnMode) emit columnSearchStarted(request);
    else emit singleSearchStarted(request);
}

void SearchWidget::doLogical() {
    if (isLocked()) return;
    lock();
    SearchWorker::SearchRequest request;
    request.type = SearchWorker::HandicapRequest;
    request.maxDepth = 0;
    request.variation = boardWidget->getVariation();

    if (columnMode) emit columnSearchStarted(request);
    else emit singleSearchStarted(request);
}

void SearchWidget::doFullWidth() {
    if (isLocked()) return;
    /*
        lock();
        SearchRequest request;
        request.type = FullWidth;
        request.maxDepth = 0;
        request.variation = boardWidget->getVariation();

        if (columnMode) emit columnSearchStarted(request);
        else emit singleSearchStarted(request);*/
}

void SearchWidget::doProofNumber() {
    if (isLocked()) return;

    lock();
    SearchWorker::SearchRequest request;
    request.type = SearchWorker::ProofNumberRequest;
    request.maxDepth = 0;
    request.variation = boardWidget->getVariation();

    if (columnMode) emit columnSearchStarted(request);
    else emit singleSearchStarted(request);
}

void SearchWidget::doExactProofNumber() {
    if (isLocked()) return;

    lock();
    SearchWorker::SearchRequest request;
    request.type = SearchWorker::ExactProofNumberRequest;
    request.maxDepth = 0;
    request.variation = boardWidget->getVariation();

    if (columnMode) emit columnSearchStarted(request);
    else emit singleSearchStarted(request);
}

void SearchWidget::doRetrograde() {
    if (isLocked()) return;

    lock();
    SearchWorker::SearchRequest request;
    request.type = SearchWorker::RetrogradeRequest;
    request.maxDepth = 0;
    request.variation = boardWidget->getVariation();

    if (columnMode) emit columnSearchStarted(request);
    else emit singleSearchStarted(request);
}

void SearchWidget::setResults(QString result) {
    textEdit->setText(result);
    available = true;
    boardWidget->allowMoves(true);
    emit searchEnded();
}

void SearchWidget::setVariation(QString var) {
    if (!computerMode) return;
    if (var.length() % 2 == 1) {
        textEdit->setText("");
        return;
    }

    assert(!isLocked());

    if (var.isEmpty() && BOARD_WIDTH == 7 && BOARD_HEIGHT == 6) {
        boardWidget->drop(3);
        return;
    }

    lock();
    SearchWorker::SearchRequest request;
    request.type = SearchWorker::HandicapRequest;
    request.maxDepth = 0;
    request.variation = boardWidget->getVariation();

    emit computerSearchStarted(request);
}

void SearchWidget::setComputerMode(bool on) {
    if (!on) {
        computerMode = false;
        boardWidget->setComputerMode(false);
        return;
    }

    if (isLocked()) return;

    boardWidget->restart();
    boardWidget->setComputerMode(true);
    computerMode = true;
    setVariation("");
}

void SearchWidget::makeComputerMove(char ch) {
    bool ok = false;

    if (ch >= 'a' && ch < 'a' + BOARD_WIDTH) {
        boardWidget->drop(ch - 'a');
        ok = true;
    } else if (ch >= 'A' && ch < 'A' + BOARD_WIDTH) {
        boardWidget->pop(ch - 'A');
        ok = true;
    }

    if (ok) {
        QString str = "Best move was: %1";
        str = str.arg(ch);
        if (!boardWidget->hasWon()) {
            int movesLeft = worker->getPlyLimit() - boardWidget->getPly();
            str += "\n\nI'll win in %1 moves or less";
            str = str.arg(movesLeft);
        }
        textEdit->setText(str);
    } else {
        textEdit->setText("Best move could not be determined! Turning off computer mode");
        computerMode = false;
        boardWidget->setComputerMode(false);
        emit computerMoveFailed();
    }

    available = true;
    boardWidget->allowMoves(true);
    emit searchEnded();
}

void SearchWidget::setText(QString text) {
    textEdit->setText(text);
}

void SearchWidget::setLamp(int column, int value) {
    boardWidget->setLamp(column, value);
    boardWidget->repaint();
}

void SearchWidget::changeDepthLimit() {
    if (isLocked()) return;

    bool ok;
    QString msg("Enter on which ply White is considered to have lost: ");
 
    int limit = QInputDialog::getInt(this, tr("Handicap depth limit"), msg, worker->getPlyLimit(), 0, 10000, 1, &ok);

    if (ok) {
        worker->setPlyLimit(limit);
        msg = "Handicap limits are:\n\nDepth = %1\nPop = %2\n\nDepth limit means the ply on which White is declared to have lost. For example if depth=1, White has to win with his next move or otherwise he is declared to have lost.\n\nPop limit is the maximum number of pop moves that White is allowed to make. For example pop=1 means that White can make only one non-winning pop move during the game. The value cannot be changed.";
        textEdit->setText(msg.arg(worker->getPlyLimit()).arg(POP_LIMIT));
    }
}
