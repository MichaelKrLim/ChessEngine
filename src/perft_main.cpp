#include "Move_generator.h"

#include <array>
#include <iostream>

using namespace engine;

int main(int argc, char* argv[])
{
	const auto depth = std::atoi(argv[1]);
	const std::string_view fen{argv[2]};
	Board base_position{fen};
	const auto for_each_move = [](const Board& board, const std::function<void(const Position&, const Position&)>&& f)
	{
		for(const auto& all_moves : legal_moves(board))
		{
			for(const auto& [origin_square, destination_squares] : all_moves)
			{
				for(const auto& destination_square : destination_squares)
				{
					f(origin_square, destination_square);
				}
			}
		}
	};
	const auto perft = [&for_each_move](this auto&& rec, int depth, const Board& board) -> unsigned long long
	{
		if(depth == 0)
			return 1ULL;
		unsigned long long nodes{0ULL};
		for_each_move(board, [&](const Position& origin_square, const Position& destination_square)
		{
			Board new_board{board};
			new_board.make(Move::make(origin_square, destination_square, Move_type::normal));
			nodes += rec(depth-1, new_board);
		});
		return nodes;
	};

	const std::array<std::unordered_map<Position, std::vector<Position>>, number_of_piece_types> initial_moves = legal_moves(base_position);
	for_each_move(base_position, [&](const Position& origin_square, const Position& destination_square)
	{
		Board new_board{base_position};
		new_board.make(Move::make(origin_square, destination_square, Move_type::normal));
		std::cout << origin_square << destination_square << ' ' << perft(depth-1, new_board) << "\n";
	});
	std::cout << "\n" << perft(depth, base_position);
}
