#ifndef search_h_INCLUDED
#define search_h_INCLUDED

#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move.h"
#include "Transposition_table.h"

#include <atomic>
#include <expected>
#include <optional>

namespace engine
{
	struct Search_results
	{
		unsigned nodes{0};
		int score{0};
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
		int hash{default_table_size};
		unsigned threads{default_threads};
		std::chrono::milliseconds move_overhead{10};
	};

	enum class timeout {};
	enum class search_stopped {};

	template <typename Io>
	[[nodiscard]]
	std::expected<Search_results, search_stopped>
	nega_max(const std::atomic<bool>& should_stop_searching
		   , const Search_options& search_options
		   , const Engine_options& engine_options
		   , State state
		   , Transposition_table& transposition_table
		   , const std::optional<Io> io=std::nullopt) noexcept;
} // namespace engine

#include "search_impl.h"

#endif // search_h_INCLUDED
