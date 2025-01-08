#ifndef FEN_h_INCLUDED
#define FEN_h_INCLUDED

#include "Pieces.h"
#include "Bitboard.h"

#include <iostream>
#include <string>

namespace engine 
{
	class FEN
	{
		public:

		[[nodiscard]] Piece to_piece(const char& to_convert) const;
		[[nodiscard]] char to_piece(const Piece& to_convert) const;
		[[nodiscard]] std::string from_board(const Board& board) const;
		[[nodiscard]] Board from_string(const std::string& str) const;
	};
}

#endif // FEN_h_INCLUDED
