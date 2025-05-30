#ifndef Move_generator_h_INCLUDED
#define Move_generator_h_INCLUDED

#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "State.h"

namespace engine
{
	enum class Moves_type
	{
		noisy, legal, size
	};

	template<Moves_type moves_type>
	[[nodiscard]] Fixed_capacity_vector<Move, max_legal_moves> generate_moves(const State& state) noexcept;
	extern template Fixed_capacity_vector<Move, max_legal_moves> generate_moves<Moves_type::legal>(const State& state) noexcept;
	extern template Fixed_capacity_vector<Move, max_legal_moves> generate_moves<Moves_type::noisy>(const State& state) noexcept;
	
	[[nodiscard]] Bitboard generate_attack_map(const State& state) noexcept;
}

#endif // Move_generator_h_INCLUDED
