#ifndef move_unmove_h_INCLUDED
#define move_unmove_h_INCLUDED

#include "nnue/Accumulator.h"
#include "Move.h"
#include "nnue/Neural_network.h"
#include "State.h"

namespace engine
{
	void make(State& state, Accumulator& accumulator, const Move& move, const Neural_network& neural_network) noexcept;
	// void make(State& state, const Move& move) noexcept;

	void unmove(State& state, Accumulator& accumulator, const Neural_network& neural_network) noexcept;
	// void unmove(State& state) noexcept;
}

#endif // move_unmove_h_INCLUDED
