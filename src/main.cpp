#include "Board.h"
#include "Engine.h"
#include "Move_generator.h"

#include <iostream>

using namespace engine;

// opening position - "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

int main()
{
	[[maybe_unused]] Engine engine;
	Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	Move_generator mg{};
	for(const auto& pieces : mg.get(board))
		for(const auto& [origin_square, destination_squares] : pieces)
		{
			for(const auto& destination : destination_squares)
			{
				std::cout << origin_square << ' ' << destination << "\n";
			}
		}
	//perft(board, 1, mg, 0, true);
	std::cerr << board.occupied_squares.pretty_string();
}
