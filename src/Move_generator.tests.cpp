#include <doctest/doctest.h>

#include "Move_generator.h"

#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

using namespace engine;

std::optional<std::string> get_diff(const std::string fen, const char depth)
{
	const std::string command{"./perftree ./perft fen "+fen+" depth "+depth};
	return {};
}

TEST_SUITE("Move_generator")
{
	TEST_CASE("Diff against stockfish")
	{
		std::optional<std::string> diff = get_diff("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", '3');
		CHECK_MESSAGE(!diff.has_value(), diff.value());
		
	}
}
