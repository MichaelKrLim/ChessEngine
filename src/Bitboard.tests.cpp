#include <doctest/doctest.h>

#include "Bitboard.h"

TEST_SUITE("Bitboard")
{
	TEST_CASE("Default Initialisation")
	{
		engine::Bitboard bitboard{};
		CHECK (bitboard == 0);
	}
}
