#include <doctest/doctest.h>

#include "Bitboard.h"

#include <cstdint>
#include <cmath>
#include <sstream>

using namespace engine;

TEST_SUITE("Bitboard")
{
	std::uint64_t data{0b1101101010101};
	Bitboard bb{data};
	TEST_CASE("Initialisation")
	{
		Bitboard default_bb{};
		CHECK (default_bb == 0);
		CHECK(bb == data);
	}
	TEST_CASE("Operators")
	{
		CHECK((bb & (101<<3)) == (data & (101<<3)));
		CHECK((bb | Bitboard{0b010110110}) == (data | 0b010110110));
	}
	TEST_CASE("Member functions")
	{
		int extracted{0};
		const auto extract_bit = [&extracted](const Position& position)
		{
			extracted += std::pow(2, to_index(position));
		};
		bb.for_each_piece(extract_bit);
		CHECK(extracted == data);
	}
	TEST_CASE("Output operator")
	{
		Bitboard bb{0b01101010};
		std::ostringstream oss{};
		oss << bb;
		CHECK(bb.pretty_string()==oss.str());
	}
}
