#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "State.h"
#include "Move.h"

#include <optional>

namespace engine
{
	[[nodiscard]] std::optional<Move> generate_move_at_depth(State state, const int depth) noexcept;
}

#endif //Engine_h_INCLUDED
