#include "bench.h"
#include "Stdio.h"
#include "Uci_handler.h"

#include <print>

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
		uci::Uci_handler uci_handler;
		uci_handler.start_listening();
	}
}
