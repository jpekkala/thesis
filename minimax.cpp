#include "game.h"
#include "minimax.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <ctime>

using namespace Connect4;

Minimax::Minimax()
: trans(NULL) {
    reportCallback = NULL;
    resetHistory();
    resetStats();
}

int Minimax::search(int depth, bool newHistory, bool newTable) {
    if (hasWon(other)) return LOSS;


#if TRANS_ON
    if (newTable) {
        trans->reset();
    }
#endif

    if (newHistory) {
        resetHistory();
    }

    resetStats();
    popCount = 0;
    resizePast(std::max(ply + depth + 1, WIDTH * HEIGHT + 100));

    clock_t begin = clock();
    int v = execute(depth);
    clock_t end = clock();
    elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;

    return v;
}

char Minimax::getBestMove(int depth) {
    if (hasWon(other)) return 0;
    resetHistory();
    Successor succ[WIDTH * 2];
    int moveCount = getSuccessors(succ);
    int quickScore = evaluateTerminals(succ, moveCount);
    if(quickScore == WIN) {
        for(int i = 0; i < moveCount; i++) {
            if(succ[i].score == WIN) {
                return succ[i].pop ? 'A' + succ[i].column : 'a' + succ[i].column;
            }
        }
    }
    
    order(succ, moveCount);

    for (int i = 0; i < moveCount; i++) {
        Successor& s = succ[i];
        if (s.pop) {
            pop(s.column);
        } else {
            drop(s.column);
        }

        int r = search(depth, true, false);
        undo();

        if (r == LOSS) {
            return s.pop ? 'A' + s.column : 'a' + s.column;
        }
    }

    return 0;
}

void Minimax::setTransTable(TransTable* tt) {
    trans = tt;
}

void Minimax::resetHistory() {
    //give middle cells a slightly better score so they are tried first in absence of everything else
    for (int x = 0; x < WIDTH; x++) {
        int v = std::min(x, WIDTH - x - 1);
        for (int y = 0; y < HEIGHT; y++) {

            dropHistory[x * H1 + y] = v;
            popHistory[x * H1 + y] = v;
        }
    }
}

void Minimax::resetStats() {

    interiorCount = terminalCount = reusedCount = inexactReusedCount = taintedCount = depthCutoffs = 0;
}

int Minimax::evaluateTerminals(Successor(&succ)[WIDTH * 2], int moveCount) {
    int best = LOSS;

    for (int i = 0; i < moveCount; i++) {
        Successor &s = succ[i];
        assert(s.score == UNKNOWN);
        //check immediate win
        if (hasWon(s.newOther)) {
            terminalCount++;
            s.score = WIN;
            return WIN;
        }
#if POPOUT_ON
        else if (s.pop && hasWon(s.newCurrent)) {
            terminalCount++;
            s.score = LOSS;
        } else {
            //check repeated position
            bitboard pos = BOTTOM + s.newCurrent + s.newCurrent + s.newOther;

            for (int j = ply - 3; j >= 0; j -= 2) {
                if (pos == pastPositions[j]) {
                    terminalCount++;
                    s.score = best = DRAW | TAINTED;
                    break;
                }
            }

            /*
             bool b = false;
             for (int j = 0; j <= ply; j++) {
                 if (pos == pastPositions[j]) {
                     b = true;
                     break;
                 }
             }

             assert(a == b);
             */

        }
#endif
    }

    //if the board is full, white can choose to draw or make a pop move
    if ((current | other) == FULL) {
        //assert(ply == WIDTH * HEIGHT);

        terminalCount++;
        best = DRAW;
    }
    return best;
}

int Minimax::order(Successor(&succ)[WIDTH * 2], int moveCount) {
    int unknown = 0;
    //count unknowns and insertion sort
    for (int i = 0; i < moveCount; i++) {
        if (succ[i].score == UNKNOWN) unknown++;

#if HISTORY_ON
        if (i == 0) continue;

        if (succ[i].pop) continue;

        int j = i;
        while (j > 0) {
            hentry a = getHistoryScore(succ[j].column);
            hentry b = getHistoryScore(succ[j - 1].column);

            if (a > b) {
                std::swap(succ[j], succ[j - 1]);
                j--;
            } else {
                break;
            }
        }
#endif
    }

    return unknown;
}