#include "Move_generator.h"
#include "Position.h"

#include <iostream>
#include <sstream>

using namespace engine;

int main(int argc, char* argv[])
{
	assert((argc == 3 || argc == 4) && "Usage: perft <depth> <fen> <moves...>");
	const auto depth = std::atoi(argv[1]);
	const std::string_view fen{argv[2]};
	State base_position{fen};
	if(argc == 4)
	{
		std::istringstream iss{argv[3]};
		std::string move{""};
		while(std::getline(iss, move, ' '))
			base_position.make(Move{algebraic_to_position(move.substr(0, 2)), algebraic_to_position(move.substr(2, 2))});
	}
	const auto perft = [](this auto&& rec, int depth, State& state, const bool&& is_root) -> unsigned long long
	{
		std::uint64_t current_count, nodes{0};
		const auto moves = legal_moves(state);
		for(const auto& move : moves)
		{
			if(is_root && depth <= 1)
			{
				current_count = 1;
				++nodes;
			}
			else
			{
				state.make(move.value());
				current_count = depth == 2? legal_moves(state).size() : rec(depth-1, state, false);
				nodes += current_count;
				state.unmove();
			}
			if(is_root)
				std::cout << move.value() << ' ' << current_count << "\n";
		}
		return nodes;
	};
	const std::uint64_t total_nodes = perft(depth, base_position, true);
	std::cout << "\n" << total_nodes;
}
