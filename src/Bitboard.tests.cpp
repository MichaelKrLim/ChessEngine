#include <doctest/doctest.h>

#include "Bitboard.h"

#include <cstdint>
#include <cmath>

TEST_SUITE("Bitboard")
{
	std::uint64_t data{0b1101101010101};
	engine::Bitboard from_uint64_t_bb{data};
	TEST_CASE("Initialisation")
	{
		engine::Bitboard default_bb{};
		CHECK (default_bb == 0);
		CHECK(from_uint64_t_bb == data);
	}
	TEST_CASE("Operators")
	{
		CHECK((from_uint64_t_bb & (101<<3)) == (data & (101<<3)));
		auto hashed_bb{from_uint64_t_bb};
		hashed_bb.hash(197);
		CHECK(hashed_bb == data*197);
	}
	TEST_CASE("Member functions")
	{
		int extracted{0};
		const auto extract_bit = [&extracted](const auto& index)
		{
			extracted += std::pow(2,index);
		};
		from_uint64_t_bb.for_each_piece(extract_bit);
		CHECK(extracted == data);
	}
}
