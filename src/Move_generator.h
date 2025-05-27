#ifndef Move_generator_h_INCLUDED
#define Move_generator_h_INCLUDED

#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "State.h"

namespace engine
{
	[[nodiscard]] Fixed_capacity_vector<Move, max_legal_moves> legal_moves(const State& state) noexcept;
	[[nodiscard]] Bitboard generate_attack_map(const State& state) noexcept;
	[[nodiscard]] Fixed_capacity_vector<Move, max_legal_moves> noisy_moves(const State& state) noexcept;
}

#endif // Move_generator_h_INCLUDED
