#include "Board.h"
#include "Engine.h"
#include "Move_generator.h"

#include <iostream>

using namespace engine;

// opening position - "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void perft(const Board& board, const int depth, const Move_generator& mg, int current_branches, bool first = false)
{
	static int total_branches{0};
	if(depth == 0)
	{
		std::cout << current_branches;
	}
	else
	{
		for(const auto& pieces : mg.get(board))
		{
			for(const auto& [origin_square, destinations] : pieces)
			{
				if(first && !destinations.empty())
					std::cout << origin_square << ' ';
				for(const auto& destination_square : destinations)
				{
					if(first) 
						std::cout << destination_square;
					if(!first) 
					{
						++current_branches;
						++total_branches;
					}
					Board next_move{board};
					next_move.make(Move::make(origin_square, destination_square, Move_type::normal));
					perft(next_move, depth-1, mg, current_branches);
				}
			}
		}
	}
	if(first) std::cout << "\n" << total_branches;
}

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
