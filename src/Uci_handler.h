#ifndef Uci_handler_h_INCLUDED
#define Uci_handler_h_INCLUDED

#include "Constants.h"

#include <chrono>
#include <optional>

namespace uci
{
	struct Search_options
	{
		std::optional<unsigned> depth{std::nullopt};
		unsigned movestogo{0};
		engine::Side_map<std::optional<std::chrono::milliseconds>> time{std::nullopt, std::nullopt};
		engine::Side_map<std::chrono::milliseconds> increment{std::chrono::milliseconds{0}, std::chrono::milliseconds{0}};
		std::optional<std::chrono::milliseconds> movetime{std::nullopt};
	};

	struct Engine_options
	{
		int hash{engine::default_table_size},
		threads{1};
		std::chrono::milliseconds move_overhead{10};
	};

	void start_listening() noexcept;
};

#endif // Uci_handler_h_INCLUDED
