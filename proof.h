
#ifndef PROOF_H
#define	PROOF_H

#include <functional>
#include "game.h"
#include "handicap.h"

typedef struct Node Node;

struct Node {
    int proof;
    int disproof;
    int value;
    bool disjunction;
    bool expanded;
    char move;
    Node* parent;
    Node* children;
    int childrenCount;
};

class Proof : public Game {
    static const int INF = 100000000;
    static const int PROVEN = 1;
    static const int DISPROVEN = 2;

    bool solveRed;

public:
    int handicapPlyLimit;
    std::function<void() > reportCallback;

    Proof() : handicapPlyLimit(Handicap::DEFAULT_PLY_LIMIT) {
    }

    int solve(bool white = true);
    int exactSolve();
    int expansions;
    int allocated;

private:
    int evaluateChild(Successor& succ);
    int evaluateChildWithHandicap(Successor& succ);
    void expand(Node*);
    void setProofNumbers(Node*);
    Node* selectMostProvingNode(Node*);
    Node* updateAncestors(Node*, Node*);

    void freeChildren(Node* node);

};


#endif	

