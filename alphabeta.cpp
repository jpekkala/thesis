#include "alphabeta.h"
#include <ctime>

using namespace Connect4;

int AlphaBeta::execute(int depth) {
    historyDepthLimit = std::max(depth - 40, 0);
    int v = negamax(depth, LOSS, WIN);
    return v & ~TAINTED;
}

int AlphaBeta::negamax(int depth, int alpha, int beta) {
#if ALPHA_BETA_ON
    assert(alpha != beta);
#endif

    bool whiteMoves = ply % 2 == 0;

    interiorCount++;

    if (reportCallback != NULL && interiorCount % 200000 == 0) {
        reportCallback();
    }

    if (depth == 0) {
        depthCutoffs++;
        return UNKNOWN;
    }

   /*
    if (popCount > 0) {
        assert(!whiteMoves);
        return WIN | TAINTED;
    }*/

    uint64_t startNodes = interiorCount;
    bitboard currentPosition = pastPositions[ply] = getPosition();
    assert(currentPosition == (((current | other) + BOTTOM) | current));

#if SYMMETRY_ON
    bool symmetric = false;
    bitboard mirror = flip(currentPosition);
    if (mirror == currentPosition) {
        symmetric = true;
    } else {
        currentPosition = std::min(currentPosition, mirror);
    }
#endif


    /**
     * bestScore is the score of this particular node
     * alpha is the score of the whole subtree so alpha >= bestScore holds always
     * a cutoff can be made only when alpha and beta both are DRAW
     */
    int bestScore = LOSS;
    bool bestTainted = false;

#if TRANS_ON
    int transScore = trans->fetch(currentPosition);

    //check if we have an exact score (see connect4.h)
    if (transScore & 1) {
        reusedCount++;
        return transScore;
    }
    //no exact score
    if (transScore != UNKNOWN) {
        inexactReusedCount++;
        //the score was neither exact nor UNKNOWN, it was stored with αβ cutoff
        if (transScore == DRAW_OR_WIN) {
            alpha = bestScore = DRAW;
        } else {
            assert(transScore == DRAW_OR_LOSS);
            beta = DRAW;
        }
#if ALPHA_BETA_ON
        //if both alpha and beta are draws, a cutoff can be made
        if (alpha == beta) return transScore;
#endif
    }
#endif

    //Step 1: Generate successors
    Successor succ[WIDTH * 2];
    int moveCount = getSuccessors(succ);

    //check for any immediate wins
    int quickScore = evaluateTerminals(succ, moveCount);
    if (quickScore == WIN) {
        return WIN;
    } else if (quickScore != LOSS) {
        if (quickScore == (DRAW | TAINTED)) bestTainted = true;
#if ALPHA_BETA_ON
#if TRANS_ON
        if (transScore == DRAW_OR_LOSS) return quickScore;
#endif
        if (beta == DRAW) {
            return bestTainted ? DRAW_OR_WIN | TAINTED : DRAW_OR_WIN;
        }
#endif
        alpha = bestScore = DRAW;
    }

    //Step 2: Order successors

    //if any of the children remains unknown, we may not have an exact score
    int unknown = order(succ, moveCount);

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
            unknown--;
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

        //the score is from the opponent's perspective now, check UNKNOWN before flipping
        score = negamax(depth - 1, SCORE_CEILING - beta, SCORE_CEILING - alpha);

        if (succ[i].pop) {
            if (whiteMoves) popCount--;
            heights[move]++;
            pastMoves[ply - 1] = 0;
        } else {
            heights[move]--;
            pastMoves[ply - 1] = 0;
        }

        if ((score & TAINTED) != 0) {
            score ^= TAINTED;
            scoreTainted = true;
            bestTainted = true;
        }

        if (score == UNKNOWN) {
            continue;
        }
        unknown--;

        //flip the score
        succ[i].score = score = SCORE_CEILING - score;

        if (score > bestScore) {
            switch (score) {
                case WIN:
                    alpha = bestScore = WIN;
                    bestTainted = scoreTainted;
                    break;
                case DRAW:
                    alpha = bestScore = DRAW;
                    break;
                case LOSS:

                    break;
                case DRAW_OR_WIN:
                    bestScore = DRAW_OR_WIN;
                    alpha = DRAW;
                    //either beta is DRAW or max search depth was reached
                    break;
                case DRAW_OR_LOSS:
                    bestScore = DRAW_OR_LOSS;
                    //either alpha is DRAW or max search depth was reached
                    break;
            }
#if ALPHA_BETA_ON
            if (alpha >= beta) {
                if (depth > historyDepthLimit) {
                    getHistoryScore(move) += (hentry) 1 << (depth - historyDepthLimit);
                }
                break;
            }
#else
            if (bestScore == WIN) break;
#endif
        }
    }
    ply--;
    current = oldCurrent;
    other = oldOther;

    //if we have unknown children left, they could potentially have improved our score
    //(but not worsened because then we simply wouldn't have chosen them)

    if (unknown > 0) {
#if ALPHA_BETA_ON
        if (bestScore == DRAW) bestScore = DRAW_OR_WIN;
        else if (bestScore < DRAW) {
            bestScore = UNKNOWN;
        }
#else
        if (bestScore < WIN) bestScore = UNKNOWN;
#endif
    }

    if (bestScore == UNKNOWN) return UNKNOWN;

#if TRANS_ON
    if (bestTainted) {
        taintedCount++;
        return bestScore | TAINTED;
    }

    if (transScore == DRAW_OR_LOSS && bestScore >= DRAW) {
        assert(bestScore != WIN);
        //we have an exact value
        bestScore = DRAW;
    }

    trans->store(currentPosition, bestScore, interiorCount - startNodes);
    //assert(trans->fetch(currentPosition) == bestScore);
#endif

    return bestScore;
}
