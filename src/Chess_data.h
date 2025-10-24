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
			constexpr int nnue_quantization_multiple{16};
			piece_values[Piece::pawn]   = 100*nnue_quantization_multiple;
			piece_values[Piece::knight] = 288*nnue_quantization_multiple;
			piece_values[Piece::bishop] = 345*nnue_quantization_multiple;
			piece_values[Piece::rook]   = 480*nnue_quantization_multiple;
			piece_values[Piece::queen]  = 1077*nnue_quantization_multiple;
			return piece_values;
		}();
	}
}

#endif
