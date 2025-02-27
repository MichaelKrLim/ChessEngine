#include <doctest/doctest.h>

#include "Position.h"

#include <sstream>

using namespace engine;

TEST_SUITE("Position")
{
	TEST_CASE("Output operator")
	{
		Position p{8, 8};
		std::ostringstream oss{};
		oss << p;
		CHECK(oss.str()=="Position{rank_: 8, file_: 8}");
	}
}
