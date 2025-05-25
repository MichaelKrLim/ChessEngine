#ifndef Uci_handler_h_INCLUDED
#define Uci_handler_h_INCLUDED

#include "Constants.h"

namespace uci
{
	struct Search_options
	{
		unsigned depth{std::numeric_limits<unsigned>::infinity()},
		movestogo,
		transposition_table_size, hash;
		engine::Side_map<unsigned> time, increment{0,0};
	};
	void start_listening() noexcept;
};

#endif // Uci_handler_h_INCLUDED
