#ifndef MINIMAX_H
#define	MINIMAX_H

#include <functional>
#include "connect4.h"
#include "game.h"
#include "transtable.h"

#define TRANS_ON 1
#define ALPHA_BETA_ON 1
#define SYMMETRY_ON 1
#define HISTORY_ON 1
//can be any non-negative integer, or -1 to turn off
#define POP_LIMIT 0

typedef uint64_t hentry;

class Minimax : public Game {
public:
    Minimax();

    virtual ~Minimax() {

    }

    int search(int depth = 0, bool newHistory = true, bool newTable = true);
    char getBestMove(int depth = 0);

    void setTransTable(TransTable*);

    uint64_t interiorCount;
    uint64_t reusedCount;
    uint64_t inexactReusedCount;
    uint64_t taintedCount;
    uint64_t terminalCount;
    uint64_t depthCutoffs;
    double elapsedSeconds;

    std::function<void() > reportCallback;

protected:
#if TRANS_ON
    TransTable* trans;
#endif
    int popCount;
    hentry dropHistory[WIDTH * (HEIGHT + 1)];
    hentry popHistory[WIDTH * (HEIGHT + 1)];

    virtual int execute(int depth) = 0;

    void resetHistory();
    void resetStats();
    int evaluateTerminals(Successor(&succ)[WIDTH * 2], int moveCount);
    int order(Successor(&succ)[WIDTH * 2], int moveCount);

    hentry& getHistoryScore(int move) {
        if (move >= 0) return dropHistory[heights[move]];
        else return popHistory[heights[-move - 1]];
        //else return popHistory[-move - 1];
    }

};



#endif

