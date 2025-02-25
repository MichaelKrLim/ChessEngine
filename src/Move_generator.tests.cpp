#include <doctest/doctest.h>

#include "Move_generator.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

using namespace engine;

std::optional<std::string> get_diff(const std::string fen, const char depth)
{
	const char* const command{std::string{"./perftree ./perft fen "+fen+" depth "+depth}.c_str()};
	std::unique_ptr<std::FILE, decltype(&pclose)> p{popen(command, "r"), &pclose};
	if(p.get() == nullptr)
		throw std::runtime_error{"Move_generator.tests.cpp: 'get_diff': failed to create pipe"};
	std::string diff(std::istreambuf_iterator<char>(p.get()), {});
	if(!diff.empty())
		return std::optional<std::string>{diff};
	return std::nullopt;
}

TEST_SUITE("Move_generator")
{
	TEST_CASE("Diff against stockfish")
	{
		Move_generator move_generator{};
		std::optional<std::string> diff = get_diff("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", '3');
		CHECK_MESSAGE(!diff.has_value(), diff.value());
	}
}
