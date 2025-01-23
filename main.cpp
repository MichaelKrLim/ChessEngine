#include "Board.h"
#include "Engine.h"

#include <iostream>

using namespace engine;

// opening position - "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
int main()
{
	Engine engine;
	Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	std::cout << board.occupied_squares.pretty_string();
}
