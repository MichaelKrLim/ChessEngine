#include "Move_generator.h"

#include <iostream>
#include <array>

using namespace engine;

int main()
{
	int depth;
	std::string fen;
	char skip;
	std::cin >> depth >> skip;
	std::getline(std::cin, fen, '"');
	Board base_position{fen};
	[[maybe_unused]]const auto count_ancestor_nodes = [](this auto&& rec, int depth, const Board& board, int num_branches) -> int
	{
		if(depth == 0)
			return num_branches;

		for(const auto& all_moves : legal_moves(board))
		{
			for(const auto& [origin_square, destination_squares] : all_moves)
			{
				for(const auto& destination_square : destination_squares)
				{
					++num_branches;
					Board new_board{board};
					new_board.make(Move::make(origin_square, destination_square, Move_type::normal));
					num_branches += rec(depth-1, new_board, num_branches);
				}
			}
		}
		return num_branches;
	};

	const std::array<std::unordered_map<Position, std::vector<Position>>, number_of_piece_types> initial_moves = legal_moves(base_position);
	for(const auto& moves : initial_moves)
	{
		for(const auto& [origin_square, destination_squares] : moves)
		{
			for(const auto& destination_square : destination_squares)
			{
				Board new_board{base_position};
				new_board.make(Move::make(origin_square, destination_square, Move_type::normal));
				std::cout << origin_square << destination_square << ' ' << count_ancestor_nodes(depth-1, new_board, 0) << "\n";
			}
		}
	}
	std::cout << count_ancestor_nodes(depth, base_position, 0) << "\n";
}
