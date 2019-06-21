#include "full.h"
#include <cassert>
#include <iostream>

using namespace Connect4;

Full::Full() : states(0), nodes(0) {

}

void Full::search() {
  
}

void Full::traverse() {
    //the method can crash because of stack size
  
    nodes++;
    if (nodes % 10000 == 0) {
        std::cout << nodes << std::endl;
    }

    bitboard pos = pastPositions[ply] = getPosition();

    //check draw
    for (int i = 0; i < ply; i++) {

        if (pastPositions[i] == pos) return;
    }

    if (hasWon(current) || hasWon(other)) {
        return;
    }

    Successor succ[WIDTH * 2];
    int moveCount = getSuccessors(succ);

    bitboard oldCurrent = current;
    bitboard oldOther = other;
    ply++;
    for (int i = 0; i < moveCount; i++) {
        current = succ[i].newCurrent;
        other = succ[i].newOther;
        traverse();
    }
    ply--;
    current = oldCurrent;
    other = oldOther;
}
