#ifndef ALPHABETA_H
#define	ALPHABETA_H

#include "minimax.h"

class AlphaBeta : public Minimax {
    int historyDepthLimit;

protected:
    virtual int execute(int depth);

private:
    int negamax(int depth, int alpha, int beta);

};


#endif

