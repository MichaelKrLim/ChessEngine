#include "Board.h"
#include "Move_generator.h"

#include <iostream>

using namespace engine;

int main()
{
	initialise_move_generator();

	Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	std::cerr << board.occupied_squares.pretty_string();
}
