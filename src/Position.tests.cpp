#include <doctest/doctest.h>

#include "Position.h"

#include <sstream>

using namespace	engine;

TEST_SUITE("Position")
{
	TEST_CASE("Output operator")
	{
		Position p{7, 7};
		std::ostringstream oss;
		oss	<< p;
		CHECK(oss.str()	== "h8");

		Position q{0, 0};
		oss.str("");
		oss	<< q;
		CHECK(oss.str()	== "a1");
	}

	TEST_CASE("Constructors")
	{
		Position p{3, 4};
		CHECK(p.rank_ == 3);
		CHECK(p.file_ == 4);

		Position q{0};
		CHECK(q.rank_ == 0);
		CHECK(q.file_ == 0);

		Position r{63};
		CHECK(r.rank_ == 7);
		CHECK(r.file_ == 7);

		auto s = algebraic_to_position("e4");
		CHECK(s.rank_ == 3);
		CHECK(s.file_ == 4);
	}

	TEST_CASE("Comparison operators")
	{
		Position p{2, 3};
		Position q{2, 3};
		Position r{3, 2};

		CHECK(p	== q);
		CHECK_FALSE(p == r);
	}

	TEST_CASE("Arithmetic operators")
	{
		Position p{1, 2};
		Position q{3, 4};
		auto sum = p + q;
		CHECK(sum.rank_	== 4);
		CHECK(sum.file_	== 6);

		p += q;
		CHECK(p.rank_ == 4);
		CHECK(p.file_ == 6);
	}

	TEST_CASE("diagonal_index and antidiagonal_index")
	{
		Position p{3, 5};
		CHECK(p.diagonal_index() ==	7 +	3 -	5);	 //	5
		CHECK(p.antidiagonal_index() ==	3 +	5);	 //	8

		Position q{0, 0};
		CHECK(q.diagonal_index() ==	7);
		CHECK(q.antidiagonal_index() ==	0);
	}

	TEST_CASE("Utility functions")
	{
		CHECK(to_index(Position{7, 7}) == 63);
		CHECK(to_index(Position{0, 0}) == 0);
		CHECK(to_index(Position{1, 1}) == 9);

		CHECK(is_on_board(Position{0, 0}));
		CHECK(is_on_board(Position{7, 7}));
		CHECK_FALSE(is_on_board(Position{-1, 0}));
		CHECK_FALSE(is_on_board(Position{0,	-1}));
		CHECK_FALSE(is_on_board(Position{8,	0}));
		CHECK_FALSE(is_on_board(Position{0,	8}));
	}
}
