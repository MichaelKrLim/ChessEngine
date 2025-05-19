#ifndef Move_generator_h_INCLUDED
#define Move_generator_h_INCLUDED

#include "State.h"

#include <vector>

namespace engine
{
	[[nodiscard]] std::vector<Move> legal_moves(const State& state) noexcept;
	[[nodiscard]] Bitboard generate_attack_map(const State& state) noexcept;
	[[nodiscard]] std::vector<Move> noisy_moves(const State& state) noexcept;
}

#endif // Move_generator_h_INCLUDED
