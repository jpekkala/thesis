#include <iostream>
#include <sstream>

#include "SearchWorker.h"
#include "connect4.h"
#include "minimax.h"
#include "full.h"
#include <functional>

SearchWorker::SearchWorker() : transTable(NULL), retro(NULL), proof(NULL) {
    alphaBeta = new AlphaBeta();
    handicap = new Handicap();
}

SearchWorker::~SearchWorker() {
    delete transTable;
    delete retro;
    delete alphaBeta;
    delete handicap;
    delete proof;
}

void SearchWorker::setTransTableSize(int size) {
    if (transTable != NULL) delete transTable;
    transTable = new TransTable(size);
    alphaBeta->setTransTable(transTable);
    handicap->setTransTable(transTable);
}

void SearchWorker::setPlyLimit(int limit) {
    handicap->plyLimit = limit;
    if (proof == NULL) proof = new Proof();
    proof->handicapPlyLimit = limit;
}

int SearchWorker::getPlyLimit() {
    int limit = handicap->plyLimit;
    if (proof != NULL) assert(limit == proof->handicapPlyLimit);
    return limit;
}

QString translateResult(int ply, int result) {
    using namespace Connect4;

    switch (result) {
        case WIN:
            return ply % 2 == 0 ? "White wins" : "White loses";
        case LOSS:
            return ply % 2 == 0 ? "White loses" : "White wins";
        case DRAW:
            return "Draw";
        case DRAW_OR_WIN:
            return ply % 2 == 0 ? "Draw or white wins" : "Draw or white loses";
        case DRAW_OR_LOSS:
            return ply % 2 == 0 ? "Draw or white loses" : "Draw or white wins";
        case DRAW_BY_REPEAT:
            return "Draw by repetition";
        case UNKNOWN:
            return "Unknown";
        default:
            return"Invalid score";
    }
}

void SearchWorker::alphaBetaReport() {
    QString str = QString::fromStdString(alphaBeta->getVariation());
    emit update(str);
}

void SearchWorker::handicapReport() {
    /*QString str = "<b>";
    std::string var = handicap->getVariation();
    for(int i = 0; i < var.length(); i++) {
        if(i % 2 == 0) {
            str.append(var.at(i));
        } else {
            str.append(QString("<font color=red>%1</font>").arg(var.at(i)));
        }
    }
    str.append("</b>");

    QString var = QString::fromStdString(handicap->getVariation());
    QString str = "Speed: %1 calls/sec\n\n%2";

    str = str.arg(handicap->reportInterval * 1000 / updateTime.elapsed()).arg(var);
    updateTime.restart();*/
    QString str = QString::fromStdString(handicap->getVariation());
    emit update(str);
}

void SearchWorker::proofNumberReport() {
    QLocale locale(QLocale::English);
    QString str = "Expanded: %1\n\nAllocated: %2\nMemory: %3 MB";
    str = str.arg(locale.toString(proof->expansions));
    str = str.arg(locale.toString(proof->allocated));
    str = str.arg(locale.toString(proof->allocated * sizeof (Node) / 1024));

    emit update(str);
}

int SearchWorker::getResult(const SearchRequest& request, const Game& game) {
    using namespace Connect4;

    int r = UNKNOWN;
    switch (request.type) {
        case AlphaBetaRequest:
            if (transTable == NULL) setTransTableSize(DEFAULT_TT_SIZE);
            alphaBeta->reportCallback = std::bind(&SearchWorker::alphaBetaReport, this);
            alphaBeta->setVariation(game.getVariation());
            r = alphaBeta->search(request.maxDepth);
            break;
        case HandicapRequest:
            if (transTable == NULL) setTransTableSize(DEFAULT_TT_SIZE);
            handicap->reportCallback = std::bind(&SearchWorker::handicapReport, this);
            handicap->setVariation(game.getVariation());
            r = handicap->search();
            break;
        case RetrogradeRequest:
            if (retro == NULL) {
                emit update("Performing retrograde analysis...");
                retro = new Retro;
            }
            r = retro->getScore(game.getPosition());
            if (r == UNKNOWN) r = DRAW_BY_REPEAT;
            break;
        case ProofNumberRequest:
            if (proof == NULL) proof = new Proof;
            proof->reportCallback = std::bind(&SearchWorker::proofNumberReport, this);
            proof->setVariation(game.getVariation());
            emit update("Doing pn-search...");
            r = proof->solve();
            break;
        case ExactProofNumberRequest:
            if (proof == NULL) proof = new Proof;
            proof->setVariation(game.getVariation());
            emit update("Doing pn-search...");
            r = proof->exactSolve();
            break;
    }
    return r;
}

void SearchWorker::processColumns(SearchRequest request) {
    using namespace Connect4;

    emit update("Solving columns");

    QTime time;
    time.start();
    Game game;
    game.setVariation(request.variation.toStdString());
    if (game.hasEnded()) {
        emit resultsReady("The game has ended");
        return;
    }

    int bestScore = LOSS;
    bool hasUnknown = false;

    try {
        for (int i = 0; i < BOARD_WIDTH; i++) {
            if (game.drop(i)) {
                int r = getResult(request, game);

                if (r == UNKNOWN) {
                    hasUnknown = true;
                } else if (r == DRAW_BY_REPEAT) {
                    bestScore = std::max(bestScore, DRAW);
                } else {
                    r = SCORE_CEILING - r;
                    bestScore = std::max(bestScore, r);
                }

                emit updateLamp(i, r);
                game.undrop(i);
            }
        }

        for (int i = 0; i < BOARD_WIDTH; i++) {
            if (game.pop(i)) {
                int r = getResult(request, game);

                if (r == UNKNOWN) {
                    hasUnknown = true;
                } else if (r == DRAW_BY_REPEAT) {
                    bestScore = std::max(bestScore, DRAW);
                } else {
                    r = SCORE_CEILING - r;
                    bestScore = std::max(bestScore, r);
                }

                emit updateLamp(i + BOARD_WIDTH, r);
                game.unpop(i);

            }
        }
    } catch (const std::bad_alloc&) {
        emit resultsReady("Search aborted: Out of memory!");
        return;
    }

    if (bestScore < WIN && hasUnknown) bestScore = UNKNOWN;

    QString str = "Result: %1\n\nVariation: %2\nTotal time: %3 ms";
    str = str.arg(translateResult(game.getPly(), bestScore));
    str = str.arg(QString::fromStdString(game.getVariation()));
    str = str.arg(time.elapsed());
    emit resultsReady(str);

}

void SearchWorker::processSingle(SearchRequest request) {
    emit update(QString("Initializing search..."));

    Game game;
    game.setVariation(request.variation.toStdString());

    QTime time;
    time.start();
    int r;
    try {
        r = getResult(request, game);
    } catch (const std::bad_alloc&) {
        emit resultsReady("Search aborted: Out of memory!");
        return;
    }

    int elapsed = time.elapsed();

    QString result = QString("Result: <b><font color=green>%1</font></b><br/><br/>Variation: %2<br/>Position id: %3<br/>");
    result = result.arg(translateResult(game.getPly(), r));
    result = result.arg(QString::fromStdString(game.getVariation()));
    result = result.arg(game.getPosition());

    if (request.type == AlphaBetaRequest || request.type == HandicapRequest) {
        Minimax* minimax;
        if (request.type == AlphaBetaRequest) {
            minimax = alphaBeta;
        } else {
            minimax = handicap;
        }

        QString str = "Interior: %1\nTerminal: %2\nReused (exact): %3\nReused (inexact): %4\nTainted: %5\nDepth cutoffs: %6\n\nTime elapsed: <b><font color=red>%7 s</font></b>\nSpeed: %8 nodes/sec";
        str = str.arg(minimax->interiorCount).arg(minimax->terminalCount);
        str = str.arg(minimax->reusedCount).arg(minimax->inexactReusedCount).arg(minimax->taintedCount);
        str = str.arg(minimax->depthCutoffs);
        str = str.arg(minimax->elapsedSeconds);
        qint64 totalNodes = minimax->interiorCount + minimax->terminalCount;
        str = str.arg(QString::number(totalNodes / (double) minimax->elapsedSeconds, 'f', 0));
        result += str.replace("\n", "<br/>");
    } else if (request.type == ProofNumberRequest) {
        QString str = "Nodes expanded: %1<br/>Time elapsed: %2 ms";
        str = str.arg(proof->expansions).arg(elapsed);
        result += str;
    } else {
        QString str = "Time elapsed: %1 ms";
        result += str.arg(elapsed);
    }

    emit resultsReady(result);
}

void SearchWorker::processComputer(SearchRequest request) {
    emit update(QString("Initializing search..."));

    Game game;
    game.setVariation(request.variation.toStdString());

    if (transTable == NULL) setTransTableSize(DEFAULT_TT_SIZE);
    handicap->reportCallback = std::bind(&SearchWorker::handicapReport, this);
    handicap->setVariation(game.getVariation());
    char ch = handicap->getBestMove();

    emit computerMove(ch);
}