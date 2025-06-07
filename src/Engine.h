#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "State.h"
#include "Move.h"
#include "Uci_handler.h"

namespace engine
{
	[[nodiscard]] Move generate_best_move(State state, const uci::Search_options&);
}

#endif //Engine_h_INCLUDED
