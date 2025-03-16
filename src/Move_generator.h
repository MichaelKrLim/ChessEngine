#ifndef Move_generator_h_INCLUDED
#define Move_generator_h_INCLUDED

#include "Board.h"

namespace engine
{
	const std::vector<Move> legal_moves(const Board& board);
}

#endif // Move_generator_h_INCLUDED
