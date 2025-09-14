#include "Move_generator.h"
#include "Pieces.h"
#include "Time_manager.h"

#include <algorithm>
#include <expected>
#include <format>
#include <future>
#include <limits>
#include <memory>
#include <thread>

namespace engine
{
	template <typename Io>
	std::expected<Search_results, search_stopped> Engine<Io>::generate_best_move(std::atomic<bool>& should_stop_searching, const Search_options& search_options) noexcept
	{
		using return_type=std::expected<Search_results, search_stopped>;
		std::atomic<bool> found_result{false};
		std::promise<return_type> shared_promise;
		std::future<return_type> future_return_value{shared_promise.get_future()};
		std::vector<std::jthread> threads;
		const auto task=[&](const std::optional<Io>& io=std::nullopt)
		{
			const auto return_value{nega_max<Io>(should_stop_searching, search_options, state_, transposition_table_, io)};
			if(!found_result.exchange(true))
				shared_promise.set_value(return_value);
			should_stop_searching=true;
		};

		for(int i{0}, extra_threads=search_options.threads-1; i<extra_threads; ++i)
			threads.emplace_back(task);
		task(io_);
		return future_return_value.get();
	}
} // namespace engine
