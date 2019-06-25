#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>
#include <ctime>

#include "proof.h"
#include "alphabeta.h"
#include "handicap.h"

using namespace std;
using namespace Connect4;

Proof game;

enum Mode {
	White, Red, Exact
};


void check(string variation, Mode mode) {
	clock_t begin, end;
	double duration;

	game.reset();
	game.setVariation(variation);

	cout << "Proof-number search " << BOARD_WIDTH << "x" << BOARD_HEIGHT << endl;
	cout << "Solving " << game.getPly() << "-ply variation: " << (game.getPly() == 0?"(empty board)" :game.getVariation()) << endl;
	bool whiteMoves = game.getPly() % 2 == 0;
	begin = clock();
	int result;
	switch(mode) {
		case White:
			result = game.solve(true);
			break;
		case Red:
			result = game.solve(false);
			break;
		case Exact:
			result = game.exactSolve();
			break;
	}
	end = clock();
	duration = (double) (end - begin) / CLOCKS_PER_SEC;
	cout << "Score: " << result << " in " << duration << " seconds" << endl;
	switch(result) {
		case WIN:
			cout << (whiteMoves?"White wins":"Red wins") << endl;
			break;
		case LOSS:
			cout << (whiteMoves?"Red wins":"White wins") << endl;
			break;
		case DRAW:
			cout << "Draw" << endl;
			break;
		case DRAW_OR_WIN:
			cout << (whiteMoves?"Draw or white wins":"Draw or red wins") << endl;
			break;
		case DRAW_OR_LOSS:
			cout << (whiteMoves?"Draw or red wins":"Draw or white wins") << endl;
			break;
		default:
			cout << "Unknown result" << endl;
	}
}

void checkFile() {
	ifstream infile("variations.txt");
	string line;
	while(std::getline(infile, line)) {
		
	}
}

void usage(char *argv[]) {
	std::cout << "Usage: " << argv[0] << " white/red/exact variation" << std::endl;
	std::cout << "Example: " << argv[0] << " white dda" << std::endl;	
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		usage(argv);
		exit(1);
	}
	Mode mode;
	if(std::strcmp(argv[1], "white") == 0) {
		mode = White;	
	}	else if(std::strcmp(argv[1], "red") == 0) {
		mode = Red;	
	}	else if(std::strcmp(argv[1], "exact") == 0) {
		mode = Exact;	
	} else {
		usage(argv);
		exit(1);
	}

	string var = "";
	
	if(argc == 3) var = argv[2];
	try {
		check(var, mode);
	}catch(const std::invalid_argument& e) {
		std::cerr << e.what() << std::endl;
	} catch(const std::bad_alloc&) {
		std::cerr << "Out of memory" << std::endl;
	} catch(...) {
		std::cerr << "Something went wrong" << std::endl;
	}
}
