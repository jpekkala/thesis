#ifndef CONNECT4_H
#define CONNECT4_H

#include <cassert>
#include <string>
#include "settings.h"

//https://github.com/qu1j0t3/fhourstones/blob/master/Game.c
// bitmask corresponds to board as follows in 7x6 case:
// . . . . . . . TOP
// 5 12 19 26 33 40 47
// 4 11 18 25 32 39 46
// 3 10 17 24 31 38 45
// 2 9 16 23 30 37 44
// 1 8 15 22 29 36 43
// 0 7 14 21 28 35 42 BOTTOM

#ifdef _MSC_VER
typedef unsigned __int64 bitboard;
typedef unsigned __int64 uint64_t;
#else
#include <inttypes.h>
typedef uint64_t bitboard;
#endif

namespace Connect4 {
    const int WIDTH = BOARD_WIDTH;
    const int HEIGHT = BOARD_HEIGHT;
    const int H1 = HEIGHT + 1;
    const int H2 = HEIGHT + 2;
    const bitboard ALL1 = ((bitboard) 1 << (H1 * WIDTH)) - 1;
    const bitboard COL1 = ((bitboard) 1 << H1) - (bitboard) 1;
    const bitboard BOTTOM = ALL1 / COL1;
    const bitboard TOP = (BOTTOM << HEIGHT);
    const bitboard FULL = ALL1 ^ TOP;

    const int UNKNOWN = 0;
    //the three exact scores (WIN, DRAW, LOSS) have the first bit set
    const int WIN = 5;
    const int DRAW = 3;
    const int LOSS = 1;
    const int DRAW_OR_WIN = 4;
    const int DRAW_OR_LOSS = 2;
    
    //(SCORE_CEILING - score) gives the value for the opponent
    const int SCORE_CEILING = 6;
    const int TAINTED = 8;
    const int DRAW_BY_REPEAT = 16;
    
    //the index of the first middle column (the left one if there are two)
    const int MIDDLE_COLUMN = (WIDTH - 1) / 2;

    inline bool hasWon(bitboard board) {
        bitboard vertical = board & (board >> 1);
        bitboard horizontal = board & (board >> H1);
        bitboard slash = board & (board >> HEIGHT);
        bitboard backslash = board & (board >> H2);

        return (vertical & (vertical >> 2)) | (horizontal & (horizontal >> 2 * H1)) | (slash & (slash >> 2 * HEIGHT)) | (backslash & (backslash >> 2 * H2));
    }

    inline bitboard getBit(int column, int height) {
        return (bitboard) 1 << (column * H1 + height);
    }

    inline bitboard getColumnMask(int column) {
        return COL1 << (H1 * column);
    }

    inline bool isLegal(bitboard board) {
        return (TOP & board) == 0;
    }

    inline bitboard getPosition(bitboard current, bitboard other) {
        return BOTTOM + current + current + other;
    }

    inline bitboard getHeightBit(const bitboard& b1, const bitboard& b2, int n) {
        //TODO: check x86 BSR instruction
        bitboard both = b1 | b2;
        both &= getColumnMask(n);
        both += (bitboard) 1 << (H1 * n);
        return both;
    }

    inline bool drop(bitboard& current, bitboard& other, int n) {
        bitboard b = current | getHeightBit(current, other, n);
        if (!isLegal(b)) return false;
        current = other;
        other = b;
        return true;
    }

    inline bool pop(bitboard& current, bitboard& other, int n) {
        bitboard bit = getBit(n, 0);
        if (!(current & bit)) return false;

        bitboard b = current ^ bit;
        current = other;
        other = b;

        bitboard column = getColumnMask(n);
        bitboard rest = ALL1 ^ column;
        current = (current & rest) | ((current & column) >> 1);
        other = (other & rest) | ((other & column) >> 1);
        return true;
    }

    inline bool undrop(bitboard& current, bitboard & other, int n) {
        bitboard h = getHeightBit(current, other, n) >> 1;
        if ((h & other) == 0) return false;
        bitboard b = current;
        current = other ^ h;
        other = b;
        return true;
    }

    inline bool unpop(bitboard& current, bitboard & other, int n) {
        bitboard column = getColumnMask(n);
        bitboard rest = ALL1 ^ column;
        bitboard bit = getBit(n, 0);

        bitboard b1 = (current & rest) | ((current & column) << 1);
        bitboard b2 = (other & rest) | ((other & column) << 1);

        if (!isLegal(b1) || !isLegal(b2)) return false;
        if (hasWon(b1) || hasWon(b2)) return false;

        current = b2 | bit;
        other = b1;
        return true;
    }

    inline bitboard flip(bitboard pos) {
        bitboard mirror = 0;
        while (pos != 0) {
            mirror = (mirror << H1) | (pos & COL1);
            pos >>= H1;
        }
        return mirror;
    }

    int countBits(bitboard board);
    std::string toString(bitboard pos);
    std::string toString(bitboard current, bitboard other);
    std::string scoreToString(int score);
}

#endif
