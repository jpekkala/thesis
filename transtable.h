#ifndef TRANS_TABLE_H
#define TRANS_TABLE_H

#include "connect4.h"

typedef uint64_t entry;

class TransTable {
    entry* table;

    //the table size also acts as a hash function so preferably it should be a prime
    unsigned int transSize;
    
    int indexSize;
    int keySize;
    int keyScoreSize;
    entry keyMask;
    entry scoreMask;
    entry workMask;
    uint64_t maxWork;

    uint64_t stored;

public:
    TransTable(unsigned int size);
    ~TransTable();
    void reset();
    void store(bitboard pos, unsigned int score, uint64_t work);
    int fetch(bitboard pos);

    int getStored() {
        return stored;
    }

};

#endif
