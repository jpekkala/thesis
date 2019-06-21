#include "handicap.h"
#include <ctime>
#include <cassert>

using namespace Connect4;

int Handicap::execute(int) {
    int v = prove();
    v &= ~TAINTED;
    if (ply % 2 == 0) {
        if (v == WIN) return WIN;
        else return UNKNOWN;
    } else {
        if (v == LOSS) return LOSS;
        else return UNKNOWN;
    }
}

/**
 * the same as evaluateTerminals but without repetition checking
 */
int Handicap::fastEvaluate(Successor(&succ)[WIDTH * 2], int moveCount) {
    int best = LOSS;

    for (int i = 0; i < moveCount; i++) {
        Successor &s = succ[i];
        assert(s.score == UNKNOWN);
        //check immediate win
        if (hasWon(s.newOther)) {
            terminalCount++;
            return WIN;
        }
#if POPOUT_ON
        else if (s.pop && hasWon(s.newCurrent)) {
            terminalCount++;
            s.score = LOSS;
        }
#endif
    }

    //if the board is full, white can choose to draw or make a pop move
    if ((current | other) == FULL) {
        terminalCount++;
        best = DRAW;
    }
    return best;
}

int Handicap::prove() {
    bool whiteMoves = ply % 2 == 0;

    interiorCount++;
    if (reportCallback != NULL && interiorCount % reportInterval == 0) {
        reportCallback();
    }

    if (ply >= plyLimit) {
        depthCutoffs++;
        return whiteMoves ? LOSS | TAINTED : WIN | TAINTED;
    }

#if POP_LIMIT != -1
    if (popCount > POP_LIMIT) {
        assert(!whiteMoves);
        return WIN | TAINTED;
    }
#endif

    uint64_t startNodes = interiorCount;
    bitboard currentPosition = pastPositions[ply] = getPosition();

#if SYMMETRY_ON
    bool symmetric = false;
    bitboard mirror = flip(currentPosition);
    if (mirror == currentPosition) {
        symmetric = true;
    } else {
        currentPosition = std::min(currentPosition, mirror);
    }
#endif

    int bestScore = LOSS;
    bool bestTainted = false;

#if TRANS_ON
    int transScore = UNKNOWN;
    //use transScore only if white hasn't made pop moves due to GHI problem
    if (popCount == 0) {
        transScore = trans->fetch(currentPosition);
    }
    if (transScore != UNKNOWN) {
        reusedCount++;
        return transScore;
    }
#endif

    //Step 1: Generate successors
    Successor succ[WIDTH * 2];
    int moveCount = getSuccessors(succ);

 #if POP_LIMIT == 0
    //repetition checking is unnecessary
    int quickScore = fastEvaluate(succ, moveCount);
#else 
    //check for any immediate wins
    int quickScore = evaluateTerminals(succ, moveCount);
#endif

    if (quickScore == WIN) {
        return WIN;
    } else if (quickScore == (DRAW | TAINTED)) {
        if (whiteMoves) {
            bestTainted = true;
        } else {
            return WIN | TAINTED;
        }
    } else if (quickScore == DRAW) {
        if (!whiteMoves) {
            return WIN;
        }
    }

    //Step 2: Order successors
    //if any of the children remains unknown, we may not have an exact score
    order(succ, moveCount);

    //Step 3: Evaluate successors
    bitboard oldCurrent = current;
    bitboard oldOther = other;

    ply++;
    for (int i = 0; i < moveCount; i++) {
        bool scoreTainted = false;
        int score = succ[i].score;
        //if the score exists, it's an exact score
        if (score != UNKNOWN) continue;

        int& move = succ[i].column;
#if SYMMETRY_ON
        if (symmetric && move > MIDDLE_COLUMN) {
            continue;
        }
#endif

        current = succ[i].newCurrent;
        other = succ[i].newOther;

        if (succ[i].pop) {
            if (whiteMoves) popCount++;
            heights[move]--;
            pastMoves[ply - 1] = 'A' + move;
        } else {
            heights[move]++;
            pastMoves[ply - 1] = 'a' + move;
        }

        score = prove();

        if (succ[i].pop) {
            if (whiteMoves) popCount--;
            heights[move]++;
            pastMoves[ply - 1] = 0;
        } else {
            heights[move]--;
            pastMoves[ply - 1] = 0;
        }

        assert(score != UNKNOWN);

        if ((score & TAINTED) != 0) {
            score ^= TAINTED;
            scoreTainted = true;
            bestTainted = true;
        }

        //flip the score
        score = SCORE_CEILING - score;
        if (score == WIN) {
            bestTainted = scoreTainted;
            bestScore = WIN;

            if (ply <= 40) {
                getHistoryScore(move) += (hentry) 1 << (40 - ply);
            }
     
            break;
        } else {
            assert(score == LOSS);
        }
    }
    ply--;
    current = oldCurrent;
    other = oldOther;

    if (bestTainted || popCount > 0) {
        taintedCount++;
        return bestScore | TAINTED;
    }

#if TRANS_ON
    trans->store(currentPosition, bestScore, interiorCount - startNodes);
    //assert(trans->fetch(currentPosition) == bestScore);
#endif

    return bestScore;
}
