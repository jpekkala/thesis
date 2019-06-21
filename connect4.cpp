#include "connect4.h"
#include <string>
#include <sstream>

namespace Connect4 {

    int countBits(bitboard board) {
        int bits = 0;
        for (; board; board >>= 1) bits += board & 1;
        return bits;
    }

    std::string toString(bitboard current, bitboard other) {
        std::string str;
        int bits = countBits(current) + countBits(other);

        bitboard white = bits % 2 == 0 ? current : other;
        bitboard black = bits % 2 != 0 ? current : other;

        for (int y = HEIGHT - 1; y >= 0; y--) {
            for (int x = 0; x < WIDTH; x++) {
                bitboard bit = getBit(x, y);
                if (white & bit) str += "X";
                else if (black & bit) str += "O";
                else str += ".";
            }
            str += "\n";
        }
        return str;
    }

    std::string toString(bitboard position) {
        int sum = 0;
        int heights[BOARD_WIDTH];
        for (int x = 0; x < BOARD_WIDTH; x++) {
            heights[x] = 0;
            for (int y = BOARD_HEIGHT; y > 0; y--) {
                if (getBit(x, y) & position) {
                    heights[x] = y;
                    sum += y;
                    break;
                }
            }
        }

        bool whiteToPlay = (sum - BOARD_WIDTH) % 2 == 0;
        std::ostringstream ss;
        ss << position << std::endl;
        std::string str = ss.str();
        
        for (int y = HEIGHT - 1; y >= 0; y--) {
            for (int x = 0; x < WIDTH; x++) {
                if (y >= heights[x]) str += ".";
                else {
                    bitboard bit = getBit(x, y);
                    str += ((position & bit) == 0) ^ whiteToPlay ? "X" : "O";
                }
            }
            str += "\n";
        }
        return str;
    }

    std::string scoreToString(int score) {
        switch (score) {
            case WIN:
                return "WIN";
            case DRAW:
                return "DRAW";
            case LOSS:
                return "LOSS";
            default:
                return "UNDEFINED";
        }
    }
}
