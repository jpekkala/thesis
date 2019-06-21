#include "game.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "connect4.h"
#include "transtable.h"

using namespace std;

Game::Game() : ply(0), pastPositions(NULL), pastMoves(NULL) {
    resizePast(1000);
    reset();
}

Game::~Game() {
    if (pastPositions != NULL) delete[] pastPositions;
    if (pastMoves != NULL) delete[] pastMoves;
}

void Game::reset() {
    current = other = 0;
    ply = 0;

    for (int i = 0; i < WIDTH; i++) {
        heights[i] = i * Connect4::H1;
        columnMasks[i] = Connect4::getColumnMask(i);
        restMasks[i] = ~columnMasks[i];
    }

    pastPositions[0] = getPosition();
}

void Game::resizePast(int limit) {
    assert(limit > ply);

    bitboard *oldPos = pastPositions;
    char* oldMoves = pastMoves;

    pastPositions = new bitboard[limit];
    pastMoves = new char[limit];
    pastSize = limit;

    if (oldPos != NULL) {
        for (int i = 0; i <= ply; i++) {
            pastPositions[i] = oldPos[i];
        }
        delete[] oldPos;
    }
    
    if(oldMoves != NULL) {
        for(int i = 0; i < ply; i++) {
            pastMoves[i] = oldMoves[i];
        }
        delete[] oldMoves;
    }

    pastMoves[ply] = 0;
    for (int i = ply + 1; i < limit; i++) {
        pastPositions[i] = 0;
        pastMoves[i] = 0;
    }
}

void Game::setVariation(const std::string& var) {
    reset();
    for (unsigned int i = 0; i < var.length(); i++) {
        play(var.at(i));
    }
}

std::string Game::getVariation() const {
    return std::string(pastMoves);
}

void Game::setPosition(bitboard pos) {
    assert(sizeof (bitboard) == 8);
    bitboard mask = pos;
    for (int i = 1; i < BOARD_HEIGHT; i++) {

        mask |= (mask >> 1) & Connect4::FULL;
    }
    mask = (mask >> 1) & Connect4::FULL;
    std::cout << Connect4::toString(mask, 0);
    ply = Connect4::countBits(mask);

    current = pos & mask;
    other = ~current & mask;

}

bool Game::hasEnded() {
    return Connect4::hasWon(current) || Connect4::hasWon(other);
}

void Game::play(char ch) {
    if (ch >= 'a' && ch < 'a' + WIDTH) {
        if (!drop(ch - 'a')) {
            std::stringstream ss;
            ss << "Invalid drop: " << ch;
            throw std::invalid_argument(ss.str());
        }
    } else if (ch >= 'A' && ch < 'A' + WIDTH) {
        if (!pop(ch - 'A')) {
            std::stringstream ss;
            ss << "Invalid pop: " << ch;
            throw std::invalid_argument(ss.str());
        }
    } else if (ch == '.') {
        if ((current | other) != Connect4::FULL) {
            throw std::invalid_argument("Invalid move: not full board");
        } else {
            pastMoves[ply] = ch;
            ply++;
        }
    } else {

        std::stringstream ss;
        ss << "Invalid move: " << ch << " (ascii code " << (int) ch << ")";
        throw std::invalid_argument(ss.str());
    }
}

void Game::undo() {
    if (ply == 0) return;
    char ch = pastMoves[ply - 1];

    if (ch >= 'a' && ch < 'a' + WIDTH) {
        undrop(ch - 'a');
    } else if (ch >= 'A' && ch < 'A' + WIDTH) {
        unpop(ch - 'A');
    } else if (ch == '.') {
        ply--;
        pastMoves[ply] = 0;
    }
}

bool Game::drop(int n) {
    if (!Connect4::drop(current, other, n)) {
        return false;
    }
    pastMoves[ply] = 'a' + n;
    ply++;
    if (ply == pastSize) {
        resizePast(2 * pastSize);
    }
    pastPositions[ply] = getPosition();
    heights[n]++;

    return true;
}

bool Game::undrop(int n) {
    if (!Connect4::undrop(current, other, n)) {
        return false;
    }
    ply--;
    heights[n]--;
    assert((current & other) == 0);
    pastMoves[ply] = 0;

    return true;
}

bool Game::pop(int n) {
    if (!Connect4::pop(current, other, n)) {
        return false;
    }
    pastMoves[ply] = 'A' + n;
    ply++;
    if (ply == pastSize) {
        resizePast(2 * pastSize);
    }
    pastPositions[ply] = getPosition();
    heights[n]--;

    return true;
}

bool Game::unpop(int n) {
    if (!Connect4::unpop(current, other, n)) {
        return false;
    }
    ply--;
    heights[n]++;
    assert((current & other) == 0);
    pastMoves[ply] = 0;

    return true;
}

int Game::getSuccessors(Successor(&succ)[WIDTH * 2], bool removeSymmetric) {
    using namespace Connect4;

    int limit = WIDTH;
    if(removeSymmetric) {
        bitboard pos = getPosition();
        if(pos == flip(pos)) limit = Connect4::MIDDLE_COLUMN + 1;
    }
    
    int moveCount = 0;
    //generate drop moves
    for (int n = 0; n < limit; n++) {
        //bitboard dropBoard = current | getHeightBit(current, other, n);
        bitboard dropBoard = current | ((bitboard) 1 << (heights[n]));

        if (isLegal(dropBoard)) {
            succ[moveCount].newCurrent = other;
            succ[moveCount].newOther = dropBoard;
            succ[moveCount].column = n;
            succ[moveCount].pop = false;
            succ[moveCount].score = UNKNOWN;
            moveCount++;
        }
    }

#if POPOUT_ON
    //generate pop moves
    for (int n = 0; n < limit; n++) {
        bitboard bit = getBit(n, 0);
        if (current & bit) {
            bitboard newCurrent = other;
            bitboard newOther = current ^ bit;
            bitboard column = getColumnMask(n);
            bitboard rest = ALL1 ^ column;
            newCurrent = (newCurrent & rest) | ((newCurrent & column) >> 1);
            newOther = (newOther & rest) | ((newOther & column) >> 1);
            succ[moveCount].newCurrent = newCurrent;
            succ[moveCount].newOther = newOther;
            succ[moveCount].column = n;
            succ[moveCount].pop = true;
            succ[moveCount].score = UNKNOWN;
            moveCount++;
        }
    }
#endif

    return moveCount;
}

bitboard Game::getPosition() const {

    return Connect4::getPosition(current, other);
}

std::string Game::toString() const {
    std::string str;
    assert((current & other) == 0);
    bitboard white = ply % 2 == 0 ? current : other;
    bitboard black = ply % 2 == 0 ? other : current;

    for (int y = HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            bitboard bit = Connect4::getBit(x, y);
            if ((white & bit) && (black & bit)) str += "B";
            else if (white & bit) str += "X";
            else if (black & bit) str += "O";
            else str += ".";
        }
        str += "\n";
    }
    return str;
}
