#include <doctest/doctest.h>

#include <string_view>

#include "Move_generator.h"

using namespace engine;

[[nodiscard]] unsigned perft(int depth, State& state, const bool&& is_root=true)
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
			current_count = depth == 2? generate_moves<Moves_type::legal>(state).size() : perft(depth-1, state, false);
			nodes += current_count;
			state.unmove();
		}
	}
	return nodes;
};

TEST_SUITE("Move Generator")
{
	struct Test_case
	{
		std::string_view name;
		std::string_view fen;
		unsigned long long expected_nodes;
	};

	const std::array<Test_case, 5> tests
	{{
		{ "Position 1", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 193690690ULL},
		{ "Position 2", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 15833292ULL},
		{ "Position 3", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 89941194ULL},
		{ "Position 4", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 674624ULL},
		{ "Position 5", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4865609ULL}
	}};

	TEST_CASE("FEN")
	{
		for(const auto& test : tests)
		{
			CAPTURE(std::string{test.name});
			State state{test.fen};
			CHECK(perft(5, state) == test.expected_nodes);
		}
	}
}
