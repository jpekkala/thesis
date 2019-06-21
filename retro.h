#ifndef RETRO_H
#define	RETRO_H

#include <deque>
#include <vector>
#include <cstring>
#include "connect4.h"

typedef struct {
    bitboard current;
    bitboard other;
} RetroNode;

typedef std::vector<RetroNode> List;

typedef unsigned char byte;


class Retro {
    static const int WIDTH = BOARD_WIDTH;
    static const int HEIGHT = BOARD_HEIGHT;

    static const byte UNINITIALIZED = 128;
    static const byte FINISHED = 64;
    static const byte HAS_DRAW = 32;
    static const byte SCORE_MASK = 15;

    uint64_t tableSize;
    byte* states;
    int stateCount;
    List terminals;

public:
    Retro();
    ~Retro();

    int getStateCount() {
        return stateCount;
    }

    int getScore(bitboard pos) {
        if((states[pos] & FINISHED) == 0) return Connect4::UNKNOWN;
        return states[pos] & SCORE_MASK;
    }
    
    int getScore(RetroNode node) {
        return getScore(Connect4::getPosition(node.current, node.other));
    }
    
    void initStates();
		void printUnreachable();
private:
    void process(List& mainList, List& popList);
    void processTerminals();
    void updateParent(byte score, bitboard, bitboard);
    void confirmValue(RetroNode);
    void confirmTree(RetroNode);
};

#endif

