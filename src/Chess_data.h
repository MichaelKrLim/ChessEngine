#ifndef Chess_data_h_INCLUDED
#define Chess_data_h_INCLUDED

#include "Pieces.h"

namespace engine
{
	namespace chess_data
	{
		constexpr static Piece_map<int> piece_values = []() constexpr
		{
			Piece_map<int> piece_values{};
			piece_values[Piece::pawn]   = 100;
			piece_values[Piece::knight] = 288;
			piece_values[Piece::bishop] = 345;
			piece_values[Piece::rook]   = 480;
			piece_values[Piece::queen]  = 1077;
			return piece_values;
		}();
	}
}

#endif
