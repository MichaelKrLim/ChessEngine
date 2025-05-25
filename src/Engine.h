#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "State.h"
#include "Move.h"
#include "Uci_handler.h"

#include <optional>

namespace engine
{
	[[nodiscard]] std::optional<Move> generate_move(State state, const uci::Search_options&) noexcept;
}

#endif //Engine_h_INCLUDED
