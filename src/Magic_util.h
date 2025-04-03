#ifndef Magic_util_h_INCLUDED
#define Magic_util_h_INCLUDED

#include "Bitboard.h"

#include <vector>

namespace engine
{
	struct Magic_square
	{
		std::vector<Bitboard> attack_table;
		std::uint64_t mask;
		std::uint64_t magic;
		std::uint8_t shift;
	};

	constexpr std::uint64_t magic_hash(const Bitboard& key, const std::uint64_t& magic, int shift) 
	{
		return (key*magic) >> shift;
	}

	constexpr std::array<Position, 4> rook_moves_ =
	{{
		Position{1, 0}, Position{0, 1},
		Position{-1, 0}, Position{0, -1}
	}};

	constexpr std::array<Position, 4> bishop_moves_ =
	{{
		Position{1,  1}, Position{1,  -1},
		Position{-1, 1}, Position{-1, -1}
	}};

	constexpr std::array<Position, 8> king_moves_ =
	{{
		Position{1,  1}, Position{1,  -1},
		Position{-1, 1}, Position{-1, -1},
		Position{1, 0}, Position{0, 1},
		Position{-1, 0}, Position{0, -1}
	}};
}

#endif // Magic_util_h_INCLUDED
