#include "Engine.h"

#include <expected>
#include <future>
#include <thread>

namespace engine
{
	std::expected<Search_results, search_stopped> Engine::generate_best_move(std::atomic<bool>& should_stop_searching, const Search_options& search_options, const bool output_diagnostics) noexcept
	{
		using return_type=std::expected<Search_results, search_stopped>;
		std::atomic<bool> found_result{false};
		std::promise<return_type> shared_promise;
		std::future<return_type> future_return_value{shared_promise.get_future()};
		std::vector<std::jthread> threads;
		const auto task=[&](const int thread_id)
		{
			const auto return_value{nega_max(should_stop_searching, search_options, state_, transposition_table_, thread_id, output_diagnostics)};
			if(!found_result.exchange(true))
				shared_promise.set_value(return_value);
			should_stop_searching=true;
		};

		for(int i{0}, extra_threads=search_options.threads-1; i<extra_threads; ++i)
			threads.emplace_back(task, i+1);
		task(search_options.threads);
		return future_return_value.get();
	}
} // namespace engine
