#ifndef Uci_handler_impl_h_INCLUDED
#define Uci_handler_impl_h_INCLUDED

#include "Constants.h"
#include "Move.h"
#include "State.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace uci
{
	template <typename Io>
	Uci_handler<Io>::Uci_handler()
	{
		worker_thread = std::jthread{[this](std::stop_token stop_token)
		{
			std::unique_lock queue_lock(queue_mtx);
			for(; !stop_token.stop_requested();)
			{
				condition_variable.wait(queue_lock, [&](){ return stop_token.stop_requested() || !task_queue.empty(); });

				if(stop_token.stop_requested())
					break;

				const auto task{std::move(task_queue.front())};
				task_queue.pop();
				queue_lock.unlock();
				task(should_stop_work);
				queue_lock.lock();
				should_stop_work=false;
			}
		}};
	}

	template <typename Io>
	std::chrono::milliseconds Uci_handler<Io>::read_time(std::istream& is) noexcept
	{
		unsigned count;
		is>>count;
		return std::chrono::milliseconds{count};
	}

	template <typename Io>
	void Uci_handler<Io>::isready_handler() noexcept
	{
		io.output("readyok");
	}

	template <typename Io>
	void Uci_handler<Io>::ucinewgame_handler() noexcept
	{
		return;
	}

	template <typename Io>
	void Uci_handler<Io>::position_handler(const Input_state& input_state) noexcept
	{
		push_task([&, input_state](const std::atomic<bool>&)
		{
			engine.clear_tt();
			engine::State state = engine::State{input_state.fen};
			for(const auto& move : input_state.continuation)
				state.make(move);
			engine.set_state(state);
		});
	}

	template <typename Io>
	void Uci_handler<Io>::go_handler(const engine::Search_options& search_options) noexcept
	{
		push_task([&, search_options](const std::atomic<bool>& stop_token)
		{
			const auto search_results=engine.generate_best_move(stop_token, search_options);
			if(!(search_results.error()==engine::Engine::search_stopped{}))
				io.output("bestmove ", search_results->pv.front());
		});
	}

	template <typename Io>
	void Uci_handler<Io>::uci_handler() noexcept
	{
		io.output(std::format("id: {}", engine::name));
		io.output(std::format("author: {}\n", engine::author));
		io.output("option name Hash type spin default 16 min 1 max 33554432");
		io.output("option name Threads type spin default 1 min 1 max 1");
		io.output("option name Move Overhead type spin default 10 min 0 max 5000");
		io.output("uciok");
	}

	template <typename Io>
	void Uci_handler<Io>::setoption_handler(const engine::Engine_options& new_engine_options) noexcept
	{
		engine.set_options(new_engine_options);
	}

	template <typename Io>
	void Uci_handler<Io>::stop_handler() noexcept
	{
		should_stop_work=true;
	}

	template <typename Io>
	void Uci_handler<Io>::start_listening() noexcept
	{
		std::string line=io.input();
		for(std::string command; line != "quit"; line=io.input())
		{
			std::istringstream iss{line};
			iss>>command;

			if(command=="debug")
				continue;

			if(const auto it{to_handler_function.find(command)}; it != to_handler_function.end())
				it->second(*this, iss);
			else
				io.output(std::format("command: {} not found", command));
		}
		worker_thread.request_stop();
		condition_variable.notify_one();
	}
} // namespace uci

#endif
