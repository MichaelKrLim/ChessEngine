#ifndef FEN_h_INCLUDED
#define FEN_h_INCLUDED

#include "Pieces.h"

#include <string>

namespace engine 
{
	struct Board;

	class FEN
	{
		public:

		FEN() = default;

		[[nodiscard]] Piece to_piece(const char& to_convert) const;
		[[nodiscard]] char to_piece(const int& to_convert) const;
		[[nodiscard]] std::string from_board(const Board& board) const;
		[[nodiscard]] Board from_string(const std::string& FEN_string) const;
	};
}

#endif // FEN_h_INCLUDED
