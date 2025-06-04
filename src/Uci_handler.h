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
		unsigned movestogo,
		hash=engine::default_table_size;
		engine::Side_map<std::optional<std::chrono::milliseconds>> time{std::nullopt, std::nullopt};
		engine::Side_map<std::chrono::milliseconds> increment{std::chrono::milliseconds{}, std::chrono::milliseconds{}};
		std::optional<std::chrono::milliseconds> movetime{std::nullopt};
	};

	void start_listening() noexcept;
};

#endif // Uci_handler_h_INCLUDED
