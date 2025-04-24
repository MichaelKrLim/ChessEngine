#ifndef Move_generator_h_INCLUDED
#define Move_generator_h_INCLUDED

#include "State.h"

#include <vector>

namespace engine
{
	[[nodiscard]] const std::vector<Move> legal_moves(const State& state) noexcept;
	[[nodiscard]] const Bitboard generate_attack_map(const State& state) noexcept;
}

#endif // Move_generator_h_INCLUDED
