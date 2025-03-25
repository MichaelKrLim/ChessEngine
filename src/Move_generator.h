#ifndef Move_generator_h_INCLUDED
#define Move_generator_h_INCLUDED

#include "Board.h"

namespace engine
{
	[[nodiscard]] const std::vector<Move> legal_moves(const Board& board) noexcept;
	[[nodiscard]] const Bitboard generate_attack_map(const Board& board, const Side& side) noexcept;
}

#endif // Move_generator_h_INCLUDED
