#include <cstdlib>
#include <iostream>

#include "alphabeta.h"
#include "handicap.h"

using namespace std;

Handicap game;

int parseColumn(char ch) {
	for (char& firstSymbol : string("1aA")) {
		if (ch >= firstSymbol && ch < firstSymbol + BOARD_WIDTH) {
			return ch - firstSymbol;
		}
	}
	return -1;
}

void check(string variation) {
	game.reset();
	for(unsigned int i = 0; i < variation.length(); i++) {
		char ch = variation.at(i);
		int x = parseColumn(ch);
		if (x == -1) {
			cout << "Invalid column: " << ch << endl;
			return;
		}
		game.drop(x);
	}

	cout << "Board size is " << BOARD_WIDTH << "x" << BOARD_HEIGHT << endl;
	cout << "Solving variation: " << variation << endl;
	
	int result = game.search((BOARD_WIDTH * BOARD_HEIGHT + 1) * 2);
	cout << "Result: " << Connect4::scoreToString(result) << " in " << game.elapsedSeconds << ", " << game.interiorCount << " nodes" << endl;
}

int main(int argc, char *argv[]) {
	TransTable *tt = new TransTable(67108859);
	game.setTransTable(tt);
	string var = "";
	if(argc == 2) var = argv[1];
	check(var);
	delete tt;
}

