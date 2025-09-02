#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Chess_data.h"
#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move.h"
#include "State.h"
#include "Transposition_table.h"

#include <chrono>
#include <expected>
#include <optional>

namespace engine
{
	struct Search_results
	{
		unsigned nodes{0};
		double score{0};
		unsigned seldepth{0};
		Fixed_capacity_vector<Move, 256> pv;
	};

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

		enum class timeout {};
		enum class search_stopped {};
		
		[[nodiscard]] std::expected<Search_results, search_stopped> generate_best_move(const std::atomic<bool>& should_stop_searching, const Search_options& search_options) noexcept;

		private:

		State state_{starting_fen};
		Engine_options options_{};
		Transposition_table transposition_table_{default_table_size};
	};
}

#endif //Engine_h_INCLUDED
