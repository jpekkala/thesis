#include "transtable.h"
#include <cstring>
#include <cassert>
#include <iostream>

int getIndexSize(int transSize) {
    int indexSize = 0;
    while (transSize > 0) {
        indexSize++;
        transSize >>= 1;
    }

    return indexSize - 1;
}

TransTable::TransTable(unsigned int size) {
    transSize = size;
    table = new entry[2 * transSize]();

    indexSize = getIndexSize(transSize);
    keySize = (BOARD_WIDTH * (BOARD_HEIGHT + 1)) - indexSize;
    if (keySize < 0) keySize = 0;
    keyScoreSize = keySize + 3;
    keyMask = ((entry) 1 << keySize) - 1;
    scoreMask = (((entry) 1 << keyScoreSize) - 1) ^ keyMask;
    workMask = ~0 ^ (keyMask | scoreMask);
    maxWork = ((uint64_t) 1 << ((sizeof (entry)*8) - keyScoreSize)) - 1;
    reset();
}

TransTable::~TransTable() {
    delete[] table;
}

/**
 * Inits transposition table to zeroes
 */
void TransTable::reset() {
    memset(table, 0, 2 * transSize * sizeof (bitboard));
    stored = 0;
}

void TransTable::store(bitboard pos, unsigned int score, uint64_t nodes) {
    assert(score <= Connect4::WIN);
    if (transSize == 0) return;

    if (nodes > maxWork) {
        std::cout << "Max work (=" << maxWork << ") exceeded: " << nodes << std::endl;
        nodes = maxWork;
    }
    stored++;
    int index = (pos % transSize)*2;
    entry key = pos >> indexSize;

    entry whole = key | ((entry) score << keySize) | (nodes << keyScoreSize);
    assert(((whole & scoreMask) >> keySize) == score);

    entry first = table[index];
    if ((first & keyMask) == key) {
        table[index] = whole;
    } else if (nodes >= (first >> keyScoreSize)) {
        table[index + 1] = first;
        table[index] = whole;
    } else {
        table[index + 1] = whole;
    }
}

int TransTable::fetch(bitboard pos) {
    if (transSize == 0) return Connect4::UNKNOWN;

    //TODO: confirm bitboard -> entry conversion
    int index = (pos % transSize)*2;
    entry key = pos >> indexSize;
    entry first = table[index];
    if ((first & keyMask) == key) {
        return (first & scoreMask) >> keySize;
    }
    entry second = table[index + 1];
    if ((second & keyMask) == key) {
        return (second & scoreMask) >> keySize;
    }
    return Connect4::UNKNOWN;
}
