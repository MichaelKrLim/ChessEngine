#include "bench.h"
#include "State.h"
#include "Uci_handler.h"

#include <print>

#include "nnue/Neural_network.h"
#include <fstream>
#include "Constants.h"

int main(int argc, const char* argv[])
{
	if(argc>1)
	{
		if(std::string_view{argv[1]}=="bench")
		{
			const auto start_time{std::chrono::steady_clock::now()};
			const unsigned total_nodes{benchmark()};
			const auto used_time{std::chrono::steady_clock::now()-start_time};
			std::println("===========================");
			std::println("Total time (ms) : {}", std::chrono::duration_cast<std::chrono::milliseconds>(used_time));
			std::println("Nodes searched  : {}", total_nodes);
			std::println("Nodes/second    : {:.0f}", total_nodes/std::chrono::duration<double>(used_time).count());
		}
		else
		{
			std::println("invalid usage");
			return 0;
		}
	}
	else
	{
		engine::State state{"rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq - 0 1"};
		std::ifstream stream{"../src/nnue/nn-c3ca321c51c9.nnue"};
		Neural_network nn{stream};
		[[maybe_unused]]
		const auto eval=nn.evaluate(state.to_halfKP_features(engine::Side::white), state.to_halfKP_features(engine::Side::black));
		std::cout<<eval<<"\n";
		uci::Uci_handler uci_handler;
		uci_handler.start_listening();
	}
}
