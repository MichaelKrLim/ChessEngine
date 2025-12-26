#ifndef search_h_INCLUDED
#define search_h_INCLUDED

#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move.h"
#include "Transposition_table.h"

#include <atomic>
#include <chrono>
#include <expected>
#include <optional>

namespace engine
{
	struct Search_results
	{
		unsigned nodes{0};
		int score{0};
		Fixed_capacity_vector<Move, 256> pv;
	};

	struct Search_options
	{
		std::optional<unsigned> depth{std::nullopt};
		unsigned movestogo{0};
		engine::Side_map<std::optional<std::chrono::milliseconds>> time{std::nullopt, std::nullopt};
		engine::Side_map<std::chrono::milliseconds> increment{std::chrono::milliseconds{0}, std::chrono::milliseconds{0}};
		std::optional<std::chrono::milliseconds> movetime{std::nullopt};
		std::chrono::milliseconds move_overhead{default_move_overhead};
		int threads{default_threads};
	};

	enum class timeout {};
	enum class search_stopped {};

	[[nodiscard]]
	std::expected<Search_results, search_stopped>
	iterative_deepening(const std::atomic<bool>& should_stop_searching
		   , const Search_options& search_options
		   , State state
		   , Transposition_table& transposition_table
		   , const int thread_id) noexcept;
} // namespace engine

#endif // search_h_INCLUDED
