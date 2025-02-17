#include <doctest/doctest.h>

#include "Move_generator.h"

using namespace engine;

TEST_SUITE("Move_generator")
{
	Move_generator move_generator{};
	TEST_CASE("Validate Reachable Square Generation")
	{
		SUBCASE("Pawns")
		{
			CHECK(1==1);
		}
		SUBCASE("Knights")
		{
			CHECK(1==1);
		}
		SUBCASE("Bishops")
		{
			CHECK(1==1);
		}
		SUBCASE("Rooks")
		{
			CHECK(1==1);
		}
		SUBCASE("King")
		{
			CHECK(1==1);
		}
	}
}
