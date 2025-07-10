#include <doctest/doctest.h>

#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

#include "Move_generator.h"

using namespace engine;

[[nodiscard]] inline unsigned long long perft(State& state, int depth, bool is_root=false)
{
    unsigned long long nodes=0;
    const auto moves=generate_moves<Moves_type::legal>(state);
    for(const auto& move : moves)
    {
        if(is_root && depth<=1)
            nodes += 1;
        else
        {
            state.make(move);
            if(depth == 2)
                nodes+=generate_moves<Moves_type::legal>(state).size();
            else
                nodes+=perft(state, depth - 1, false);
            state.unmove();
        }
    }
    return nodes;
}

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
        { "Start Position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 197281ULL },
        { "Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4085603ULL },
        { "Skipe Engine", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 43238ULL },
        { "Kiwipete EP", "r3k2r/P1pp1pb1/bn2Qnp1/3PN3/1p2P3/2N2q1p/PPPBBPPP/R3K2R b KQkq e3 0 1", 382891ULL },
        { "Complex EP", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2103487ULL }
	}};

	TEST_CASE("FEN")
	{
		for(const auto& test : tests)
		{
			CAPTURE(std::string{test.name});
			State state{test.fen};
			CHECK(perft(state, 4, true) == test.expected_nodes);
		}
	}
}