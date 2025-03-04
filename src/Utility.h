#ifndef Utility_h_INCLUDED
#define Utility_h_INCLUDED

#include "Bitboard.h"
#include "Position.h"

namespace engine
{
	constexpr bool is_valid_destination(const Position& square, const Bitboard& occupied_squares)
	{
		if(!is_on_board(square))
			return false;

		bool square_is_occupied = !(occupied_squares & (1 << to_index(Position(square))));
		return square_is_occupied;
	}
}

#endif
