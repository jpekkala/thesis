#ifndef HANDICAP_H
#define	HANDICAP_H

#include "minimax.h"

class Handicap : public Minimax {
public:
    
#if BOARD_WIDTH == 7 && BOARD_HEIGHT == 6
    static const int DEFAULT_PLY_LIMIT = 21;
#else
    static const int DEFAULT_PLY_LIMIT = WIDTH * HEIGHT;
#endif
  
    int plyLimit;
    static const int reportInterval = 200000;

    Handicap() : plyLimit(DEFAULT_PLY_LIMIT) {
    };

protected:
    int execute(int depth);

private:
    int prove();
    int fastEvaluate(Successor(&succ)[WIDTH * 2], int moveCount);

};


#endif

