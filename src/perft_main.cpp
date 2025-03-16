#include "Move_generator.h"
#include "Position.h"

#include <iostream>

using namespace engine;

int main(int argc, char* argv[])
{
	const auto depth = std::atoi(argv[1]);
	const std::string_view fen{argv[2]};
	Board base_position{fen};
	for(int i{3}; i<argc; ++i)
	{
		const std::string move = argv[i];
		base_position.make(Move{algebraic_to_position(move.substr(0, 2)), algebraic_to_position(move.substr(2, 2))});
	}
	const auto perft = [](this auto&& rec, int depth, Board& board) -> unsigned long long
	{
		if(depth == 0)
			return 1ULL;
		unsigned long long nodes{0ULL};
		for(const auto& move : legal_moves(board))
		{
			board.make(move);
			nodes += rec(depth-1, board);
			board.unmove();
		}
		return nodes;
	};

	for(const auto& move : legal_moves(base_position))
	{
		base_position.make(move);
		std::cout << move.from_square() << move.destination_square() << ' ' << perft(depth-1, base_position) << "\n";
		base_position.unmove();
	}
	std::cout << "\n" << perft(depth, base_position);
}
