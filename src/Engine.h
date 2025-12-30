#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Constants.h"
#include "search.h"
#include "State.h"
#include "Transposition_table.h"

#include <expected>

namespace engine
{
	class Engine
	{
		public:

		inline void clear_tt() noexcept
		{
			transposition_table_.clear();
		}

		inline void set_state(const State& state) noexcept
		{
			state_=state;
		}

		inline void resize_tt(int size_mb) noexcept
		{
			transposition_table_.resize(size_mb);
		}

		[[nodiscard]] std::expected<Search_results, search_stopped> generate_best_move(std::atomic<bool>& should_stop_searching, const Search_options& search_options) noexcept;

		Neural_network neural_network{"/home/michael/coding/projects/ChessEngine/src/nnue/nn-97f742aaefcd.nnue"};

		private:

		State state_{starting_fen, neural_network};
		Transposition_table transposition_table_{default_table_size};
	};
}

#endif //Engine_h_INCLUDED
