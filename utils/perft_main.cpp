#include "Move_generator.h"
#include "move_unmove.h"
#include "nnue/Neural_network.h"

#include <iostream>
#include <sstream>
#include <string_view>
#include <tuple> // std::ignore

using namespace engine;

const auto perft = [](this auto&& rec, int depth, State& state, const bool&& is_root, const auto& do_move, const auto& undo_move, const auto& evaluator) -> unsigned long long
{
	unsigned long long current_count{0}, nodes{0};
	const auto moves{generate_moves<Moves_type::legal>(state)};
	for(const auto& move : moves)
	{
		if(is_root && depth <= 1)
		{
			current_count=1;
			++nodes;
		}
		else
		{
			do_move(state, move);

			if(depth==2)
			{
				evaluator(state);
				current_count=generate_moves<Moves_type::legal>(state).size();
			}
			else
				current_count=rec(depth-1, state, false, do_move, undo_move, evaluator);

			nodes+=current_count;

			undo_move(state);
		}
		if(is_root)
			std::cout<<move<<' '<<current_count<<std::endl;
	}
	return nodes;
};

int main(int argc, char* argv[])
{
	assert((argc == 3 || argc == 4) && "Usage: perft <depth> <fen> <with nn> <moves...>");

	const auto depth = std::atoi(argv[1]);

	const std::string_view fen{std::string_view{argv[2]}=="startpos"? engine::starting_fen:argv[2]};
	State base_position{fen};

	const bool use_nn{static_cast<bool>(*argv[3]-'0')};

	if(argc == 5)
	{
		std::istringstream iss{argv[4]};
		for(Move move; iss>>move;)
			make(base_position, move);
	}

	unsigned long long total_nodes{0};
	if(!use_nn)
	{
		total_nodes=perft(depth
						, base_position
						, true
						, [](State& state, const Move& move){ make(state, move); }
						, [](State& state){ unmove(state); }
						, [](State&){ return; });
	}
	else
	{
		const Neural_network neural_network{"/home/michael/coding/projects/ChessEngine/src/nnue/nn-97f742aaefcd.nnue"};
		Accumulator accumulator{fresh_accumulator(base_position, neural_network)};
		total_nodes=perft(depth
						, base_position
						, true
						, [&](State& state, const Move& move){ make(state, accumulator, move, neural_network); }
						, [&](State& state){ unmove(state, accumulator, neural_network); }
						, [&](State& state){ std::ignore=state.evaluate(neural_network, accumulator); });
		
	}

	std::cout<<"\n"<<total_nodes;
}
