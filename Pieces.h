#ifndef Pieces_h_INCLUDED
#define Pieces_h_INCLUDED

#include <cstdint>

enum class Piece : std::uint8_t
{
	pawn = 0, knight = 1, bishop = 2, rook = 3, queen = 4, king = 5
};

#endif // Pieces_h_INCLUDED
