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
		return !occupied_squares.is_occupied(square);
	}
}

#endif
