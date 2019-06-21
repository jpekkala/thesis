#include "connect4.h"
#include "retro.h"
#include "game.h"
#include <iostream>
#include <ctime>

using namespace std;
using namespace Connect4;

int main() {
	std::cout << "Solving " << BOARD_WIDTH << "x" << BOARD_HEIGHT << " by retrograde analysis" << std::endl;
	clock_t begin, end;
	begin = clock();
	Retro retro;
	end = clock();
	double duration = (double) (end - begin) / CLOCKS_PER_SEC;
	cout << "Duration: " << duration << " seconds" << endl;
	Game game;
	for(int i = 0; i < BOARD_WIDTH; i++) {
		game.drop(i);
		std::cout << (char) ('a' + i) << "=" << retro.getScore(game.getPosition()) << " ";
		game.undo();
	}
	cout << endl;
	system("pause");
}
