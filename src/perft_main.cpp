#include "Move_generator.h"

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
		for(Move move; iss>>move;)
			base_position.make(move);
	}
	const auto perft = [](this auto&& rec, int depth, State& state, const bool&& is_root) -> unsigned long long
	{
		unsigned long long current_count{0}, nodes{0};
		const auto moves = generate_moves<Moves_type::legal>(state);
		for(const auto& move : moves)
		{
			if(is_root && depth <= 1)
			{
				current_count=1;
				++nodes;
			}
			else
			{
				state.make(move);
				current_count = depth == 2? generate_moves<Moves_type::legal>(state).size() : rec(depth-1, state, false);
				nodes += current_count;
				state.unmove();
			}
			if(is_root)
				std::cout << move << ' ' << current_count << "\n";
		}
		return nodes;
	};
	const auto total_nodes = perft(depth, base_position, true);
	std::cout << "\n" << total_nodes;
}
