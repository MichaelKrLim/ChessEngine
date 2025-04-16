#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Board.h"
#include "Move.h"

namespace engine
{
	[[nodiscard]] Move generate_move_at_depth(Board board, const int depth) noexcept;
}

#endif //Engine_h_INCLUDED
