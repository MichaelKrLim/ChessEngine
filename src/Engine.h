#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Chess_data.h"
#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move.h"
#include "search.h"
#include "State.h"
#include "Transposition_table.h"

#include <chrono>
#include <expected>
#include <optional>

namespace engine
{
	template <typename Io>
	class Engine
	{
		public:

		inline void clear_tt() noexcept
		{
			transposition_table_.clear();
		}

		inline void resize_tt(int size_mb) noexcept
		{
			transposition_table_.resize(size_mb);
		}

		inline void set_options(const Engine_options& options) noexcept
		{
			options_=options;
		}

		inline void set_state(const State& state) noexcept
		{
			state_=state;
		}

		[[nodiscard]] std::expected<Search_results, search_stopped> generate_best_move(std::atomic<bool>& should_stop_searching, const Search_options& search_options) noexcept;

		private:

		Io io_;
		State state_{starting_fen};
		Engine_options options_{};
		Transposition_table transposition_table_{default_table_size};
	};
}

#include "Engine_impl.h"

#endif //Engine_h_INCLUDED
