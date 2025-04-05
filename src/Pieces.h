#ifndef Pieces_h_INCLUDED
#define Pieces_h_INCLUDED

#include "Enum_map.h"

#include <cstdint>

namespace engine
{
	enum class Piece : std::uint8_t
	{
		king = 0, pawn = 1, knight = 2, bishop = 3, rook = 4, queen = 5, size = 6
	};
	template <typename Mapped_type>
	using Piece_map = Enum_map_from_size<Piece, Mapped_type>;
	const std::size_t number_of_piece_types = 6;
}

#endif // Pieces_h_INCLUDED
