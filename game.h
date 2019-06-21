#ifndef GAME_H
#define GAME_H

#include <string>
#include "connect4.h"

typedef struct {
    int column;
    bool pop;
    bitboard newCurrent;
    bitboard newOther;
    int score;
} Successor;

class Game {
protected:
    static const int WIDTH = BOARD_WIDTH; //BOARD_WIDTH and BOARD_HEIGHT are defined in connect4.h
    static const int HEIGHT = BOARD_HEIGHT;

    int heights[BOARD_WIDTH];
    bitboard columnMasks[BOARD_WIDTH];
    bitboard restMasks[BOARD_WIDTH];

    bitboard current;
    bitboard other;

    int ply;

    int pastSize;
    bitboard* pastPositions;
    char* pastMoves;
    void resizePast(int);
    int getSuccessors(Successor(&succ)[WIDTH * 2], bool removeSymmetric = false);

public:
    Game();
    ~Game();
    std::string toString() const;
    std::string toString(bitboard);
    void reset();
    void play(char ch);
    void undo();
    void setVariation(const std::string& variation);
    std::string getVariation() const;

    bool drop(int n);
    bool pop(int n);
    bool undrop(int n);
    bool unpop(int n);
    bitboard getPosition() const;
    void setPosition(bitboard);
    bool hasEnded();

    int getPly() const {
        return ply;
    }
};


#endif
