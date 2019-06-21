#include "proof.h"
#include <algorithm>
#include <iostream>
#include <cassert>

using namespace Connect4;

int Proof::solve(bool white) {
    solveRed = !white;
    bool solverMoves = (ply % 2 == 0) ^ solveRed;

    if (Connect4::hasWon(other)) {
        return LOSS;
    }
    if (Connect4::hasWon(current)) {
        return WIN;
    }

    expansions = allocated = 0;
    Node root;
    //root.disjunction = true;
    root.disjunction = solverMoves;
    root.expanded = false;
    root.parent = NULL;
    root.value = UNKNOWN;
    setProofNumbers(&root);
    Node* current = &root;
    while (root.proof != 0 && root.disproof != 0) {
        Node* mostProving = selectMostProvingNode(current);
        expand(mostProving);
        current = updateAncestors(mostProving, &root);
    }

    freeChildren(&root);
    if (root.proof == 0) {
        return solverMoves ? WIN : LOSS;
    }
    if (root.disproof == 0) {
        return solverMoves ? DRAW_OR_LOSS : DRAW_OR_WIN;
    }
    return UNKNOWN;
}

int Proof::exactSolve() {
    int white = solve(true);
    if ((white & 1) != 0) {
        return white;
    }

    int red = solve(false);
    if ((red & 1) != 0) {
        return red;
    }

    if (red == UNKNOWN || white == UNKNOWN) {
        return UNKNOWN;
    }

    return DRAW;
}

Node* Proof::selectMostProvingNode(Node* n) {
    while (n->expanded) {
        Node* best = NULL;
        if (n->disjunction) {
            for (int i = 0; i < n->childrenCount; i++) {
                Node* c = &n->children[i];
                if (n->proof == c->proof) {
                    best = c;
                    break;
                }
            }
        } else {
            for (int i = 0; i < n->childrenCount; i++) {
                Node* c = &n->children[i];
                if (n->disproof == c->disproof) {
                    best = c;
                    break;
                }
            }
        }
        assert(best != NULL);
        n = best;

        play(n->move);
    }
    return n;
}

void Proof::setProofNumbers(Node* n) {

    if (n->expanded) {
        if (n->disjunction) {
            //OR node
            n->proof = INF;
            n->disproof = 0;
            for (int i = 0; i < n->childrenCount; i++) {
                Node& c = n->children[i];
                n->disproof += c.disproof;
                n->proof = std::min(n->proof, c.proof);
            }
        } else {
            //AND node
            n->proof = 0;
            n->disproof = INF;
            for (int i = 0; i < n->childrenCount; i++) {
                Node& c = n->children[i];
                n->proof += c.proof;
                n->disproof = std::min(n->disproof, c.disproof);
            }
        }
    } else {
        switch (n->value) {
            case PROVEN:
                n->proof = 0;
                n->disproof = INF;
                break;
            case DISPROVEN:
                n->proof = INF;
                n->disproof = 0;
                break;
            default:
                n->proof = 1;
                n->disproof = 1;
        }
    }

    if (n->proof == 0 || n->disproof == 0) {
        freeChildren(n);
    }
}

int Proof::evaluateChildWithHandicap(Successor& s) {
    assert(!solveRed);

    bool whiteMoves = ply % 2 == 0;

    if (hasWon(s.newOther)) {
        return whiteMoves? PROVEN : DISPROVEN;
    }

    if (s.pop) {
        if (hasWon(s.newCurrent)) {
            return whiteMoves? DISPROVEN : PROVEN;
        }
        if (whiteMoves) return DISPROVEN;
    }

    if (ply >= handicapPlyLimit) return DISPROVEN;
    return UNKNOWN;
}

int Proof::evaluateChild(Successor& s) {

    bool whiteMoves = ply % 2 == 0;

    int value = UNKNOWN;
    if (hasWon(s.newOther)) {
        //value = parent->disjunction ? PROVEN : DISPROVEN;
        value = whiteMoves ^ solveRed ? PROVEN : DISPROVEN;
    } else if (s.pop && hasWon(s.newCurrent)) {
        //value = parent->disjunction ? DISPROVEN : PROVEN;
        value = whiteMoves ^ solveRed ? DISPROVEN : PROVEN;
    } else {
        //check repeated position
        bitboard pos = BOTTOM + s.newCurrent + s.newCurrent + s.newOther;
        for (int j = ply - 3; j >= 0; j -= 2) {
            if (pos == pastPositions[j]) {
                value = DISPROVEN;
                break;
            }
        }
    }
    return value;
}

void Proof::expand(Node* parent) {
    expansions++;
    if (reportCallback != NULL && expansions % 100000 == 0) {
        reportCallback();
    }

    if (parent->move == '.') {
        parent->expanded = true;
        return;
    }

    Successor succ[WIDTH * 2];
    int count = getSuccessors(succ, false);

    if ((current | other) == FULL) {
        parent->childrenCount = count + 1;
        parent->children = new Node[count + 1];

        Node& n = parent->children[count];
        n.value = DISPROVEN;
        n.move = '.';
        n.disjunction = !parent->disjunction;
        n.childrenCount = 0;
        n.expanded = false;
        setProofNumbers(&n);
    } else {
        parent->childrenCount = count;
        parent->children = new Node[count];
    }
    allocated += parent->childrenCount;

    for (int i = 0; i < count; i++) {
        Successor& s = succ[i];

        Node& n = parent->children[i];
        n.value = evaluateChildWithHandicap(s);
        n.move = s.pop ? 'A' + s.column : 'a' + s.column;
        n.disjunction = !parent->disjunction;
        n.expanded = false;
        n.parent = parent;
        n.childrenCount = 0;

        setProofNumbers(&n);
    }
    parent->expanded = true;
}

Node* Proof::updateAncestors(Node *n, Node* root) {
    while (n != root) {
        int oldProof = n->proof;
        int oldDisproof = n->disproof;
        setProofNumbers(n);
        if (n->proof == oldProof && n->disproof == oldDisproof) return n;
        n = n->parent;
        undo();
    }
    setProofNumbers(root);
    return root;
}

void Proof::freeChildren(Node* node) {
    if (node->childrenCount == 0) return;

    for (int i = 0; i < node->childrenCount; i++) {
        freeChildren(&node->children[i]);
    }
    allocated -= node->childrenCount;
    delete[] node->children;
    node->childrenCount = 0;
}
