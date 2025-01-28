#ifndef Pieces_h_INCLUDED
#define Pieces_h_INCLUDED

#include <cstdint>

namespace engine
{
	enum class Piece : std::uint8_t
	{
		king = 0, pawn = 1, knight = 2, bishop = 3, rook = 4, queen = 5
	};

	const std::size_t number_of_piece_types = 6;
}

#endif // Pieces_h_INCLUDED
