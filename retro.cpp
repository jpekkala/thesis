#include "retro.h"
#include <iostream>
#include <queue>
#include <new>
#include <cassert>

using namespace Connect4;

bitboard parentPos;

Retro::Retro() {
    tableSize = (uint64_t) 1 << (WIDTH * (HEIGHT + 1));
    states = new byte[tableSize];

    try {
        initStates();
        processTerminals();
    } catch (std::bad_alloc& ba) {
        std::cerr << "bad_alloc: " << ba.what() << std::endl;
        std::cout << "States generated: " << stateCount << std::endl;
        exit(1);
    }

    byte rootScore = states[Connect4::getPosition(0, 0)];
    if ((rootScore & FINISHED) == 0) {
        std::cout << "Root score: Draw by repeat" << std::endl;
    } else {
        std::cout << "Root score: " << scoreToString(rootScore & SCORE_MASK) << std::endl;
    }
    
}

Retro::~Retro() {
    delete[] states;
}

void Retro::initStates() {
    memset(states, 0, tableSize * sizeof (byte));
    for (unsigned int i = 0; i < tableSize; i++) {
        states[i] = UNINITIALIZED;
    }

    stateCount = 0;
    List dropList;
    dropList.reserve(1000000);
    List popList;

    RetroNode root;
    root.current = 0;
    root.other = 0;
    dropList.push_back(root);
    states[Connect4::getPosition(root.current, root.other)] = 0;
  
    process(dropList, popList);
    std::cout << "Drop states: " << stateCount << std::endl;
    assert(dropList.empty());
    List(dropList).swap(dropList);

    process(popList, popList);
    std::cout << "Pop states: " << stateCount << std::endl;
    assert(popList.empty());
}

void Retro::process(List& mainList, List& popList) {
    using namespace Connect4;

    while (!mainList.empty()) {
        RetroNode currentNode = mainList.back();
        bitboard currentPosition = parentPos = getPosition(currentNode.current, currentNode.other);
        mainList.pop_back();
        stateCount++;

        if (hasWon(currentNode.other)) {
            states[currentPosition] = FINISHED | LOSS;
            terminals.push_back(currentNode);
            continue;
        } else if (hasWon(currentNode.current)) {
            states[currentPosition] = FINISHED | WIN;
            terminals.push_back(currentNode);
            continue;
        }
        for (int x = 0; x < WIDTH; x++) {
            RetroNode node = currentNode;
            if (drop(node.current, node.other, x)) {
                bitboard pos = getPosition(node.current, node.other);
                if (states[pos] == UNINITIALIZED) {
                    states[pos] = 0;
                    mainList.push_back(node);
                }
                states[currentPosition]++;
            }
#if POPOUT_ON
            node = currentNode;
            if (pop(node.current, node.other, x)) {
                bitboard pos = getPosition(node.current, node.other);
                if (states[pos] == UNINITIALIZED) {
                    states[pos] = 0;
                    popList.push_back(node);
                }
                states[currentPosition]++;
            }
#endif
        }
        //check for full board
        if ((currentNode.current | currentNode.other) == FULL) {
            if (states[currentPosition] == 0) {
                states[currentPosition] = FINISHED | DRAW;
                terminals.push_back(currentNode);
            } else {
                states[currentPosition] |= HAS_DRAW;
            }
        }
    }
}

void Retro::updateParent(byte childScore, bitboard current, bitboard other) {
    using namespace Connect4;
    bitboard pos = getPosition(current, other);
    if (pos > Connect4::ALL1) {
        std::cout << Connect4::toString(parentPos) << std::endl;
        std::cout << parentPos << std::endl;
        std::cout << Connect4::toString(pos);
        assert(pos <= ALL1);
    }
    if ((states[pos] & UNINITIALIZED) != 0) return;
    if ((states[pos] & FINISHED) != 0) return;

    if (childScore == LOSS) {
        states[pos] = FINISHED | WIN;
        RetroNode n;
        n.current = current;
        n.other = other;
        terminals.push_back(n);
        //confirmValue(n);
        return;
    }

    if (childScore == DRAW) {
        states[pos] |= HAS_DRAW;
    }

    assert((states[pos] & SCORE_MASK) > 0);
    states[pos]--;

    if ((states[pos] & SCORE_MASK) == 0) {
        if ((states[pos] & HAS_DRAW) != 0) {
            states[pos] = FINISHED | DRAW;
        } else {
            states[pos] = FINISHED | LOSS;
        }
        RetroNode n;
        n.current = current;
        n.other = other;
        terminals.push_back(n);
        //confirmValue(n);
    }
}

void Retro::processTerminals() {
    using namespace Connect4;

    while (!terminals.empty()) {
        RetroNode currentNode = terminals.back();
        terminals.pop_back();
        bitboard currentPosition = parentPos = getPosition(currentNode.current, currentNode.other);

        byte score = states[currentPosition] & SCORE_MASK;

        for (int x = 0; x < WIDTH; x++) {
            RetroNode node = currentNode;
            if (undrop(node.current, node.other, x)) {
                updateParent(score, node.current, node.other);
            }

#if POPOUT_ON
            node = currentNode;
            if (unpop(node.current, node.other, x)) {
                updateParent(score, node.current, node.other);
            }
#endif
        }
    }
}

void Retro::confirmValue(RetroNode node) {

    int parentScore = getScore(node);
    if (hasWon(node.other)) {
        assert(parentScore == LOSS);
        return;
    }
    if (hasWon(node.current)) {
        if (parentScore != WIN) {
            std::cout << toString(node.current, node.other) << std::endl;
            std::cout << "Score was: " << parentScore << " (" << scoreToString(parentScore)  << ")" << std::endl;
            std::cout << "Should be: " << scoreToString(WIN) << std::endl;
            assert(parentScore == WIN);
        }
        return;
    }

    if((states[getPosition(node.current, node.other)] & FINISHED) == 0) return;
    
    if (parentScore == WIN) {
        bool hasLosing = false;
        for (int i = 0; i < WIDTH; i++) {
            RetroNode n = node;
            if (drop(n.current, n.other, i)) {
                if (getScore(n) == LOSS) {
                    hasLosing = true;
                    break;
                }
            }
            n = node;
            if (pop(n.current, n.other, i)) {
                if (getScore(n) == LOSS) {
                    hasLosing = true;
                    break;
                }
            }
        }
        assert(hasLosing);
    } else {
        bool hasDraw = false;
        for (int i = 0; i < WIDTH; i++) {
            RetroNode n = node;
            if (drop(n.current, n.other, i)) {
                if (getScore(n) == LOSS) {
                    std::cout << toString(node.current, node.other) << std::endl;
                    std::cout << toString(n.current, n.other);
                    std::cout << parentScore;
                    assert(getScore(n) != LOSS);
                }
                if (getScore(n) == DRAW) {
                    hasDraw = true;
                }
            }
            n = node;
            if (pop(n.current, n.other, i)) {
                assert(getScore(n) != LOSS);
                if (getScore(n) == DRAW) {
                    hasDraw = true;
                }
            }
        }
        if ((node.current | node.other) == FULL) hasDraw = true;

        if (parentScore == DRAW && !hasDraw) {
            std::cout << toString(node.current, node.other);
            exit(19);
        }
    }
}


void Retro::confirmTree(RetroNode rootNode) {
    List list;
    list.push_back(rootNode);
    while (!list.empty()) {
        RetroNode node = list.back();
        list.pop_back();
        confirmValue(node);
        if(hasWon(node.current) || hasWon(node.other)) continue;
        for (int i = 0; i < WIDTH; i++) {
            RetroNode n = node;
            if (drop(n.current, n.other, i)) {
                list.push_back(n);
            }
        }
#if POPOUT_ON
        for (int i = 0; i < WIDTH; i++) {
            RetroNode n = node;
            if (pop(n.current, n.other, i)) {
                list.push_back(n);
            }
        }
#endif
    }
}

void Retro::printUnreachable() {
	for(bitboard pos = BOTTOM; pos < tableSize; pos++) {
		if(states[pos] !=	UNINITIALIZED) continue;

	}
}
