#ifndef Uci_handler_impl_h_INCLUDED
#define Uci_handler_impl_h_INCLUDED

#include "Constants.h"
#include "Move.h"
#include "State.h"
#include "search.h"

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
			for(; !stop_token.stop_requested();)
			{
				const auto task{pop_task(stop_token)};
				if(!task)
					break;
				(*task)(should_stop_work);
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
		push_task([this, input_state](std::atomic<bool>&)
		{
			engine.clear_tt();
			engine::State state = engine::State{input_state.fen};
			for(const auto& move : input_state.continuation)
				state.make(move);
			engine.set_state(state);
		});
	}

	template <typename Io>
	void Uci_handler<Io>::go_handler(const Go_options& go_options) noexcept
	{
		push_task([this, go_options](std::atomic<bool>& stop_token)
		{
			engine::Search_options search_options {
				.depth=go_options.depth,
				.movestogo=go_options.movestogo,
				.time=go_options.time,
				.increment=go_options.increment,
				.movetime=go_options.movetime,
				.move_overhead=options.move_overhead,
				.threads=options.threads
			};
			const auto search_results=engine.generate_best_move(stop_token, search_options);
			if(!(search_results.error()==engine::search_stopped{}))
				io.output("bestmove ", search_results->pv.front());
		});
	}

	template <typename Io>
	void Uci_handler<Io>::uci_handler() noexcept
	{
		io.output(std::format("id: {}", engine::name));
		io.output(std::format("author: {}\n", engine::author));
		io.output("option name Hash type spin default 16 min 1 max 33554432");
		io.output(std::format("option name Threads type spin default {} min 1 max 1024", engine::default_threads));
		io.output("option name Move Overhead type spin default 10 min 0 max 5000");
		io.output("uciok");
	}

	template <typename Io>
	void Uci_handler<Io>::setoption_handler(const Uci_option& uci_option) noexcept
	{
		if(uci_option.name=="Hash")
		{
			if(int value=std::stoi(uci_option.value); value>0 && value<max_table_size)
				engine.resize_tt(value);
			else
				io.output("In setoption name 'Hash': value out of range");
		}
		else if(uci_option.name=="Threads")
		{
			if(int threads{std::stoi(uci_option.value)}; threads > 0 && threads < 1025)
				options.threads=std::stoi(uci_option.value);
			else
				io.output("In setoption name 'Threads': value out of range");
		}
		else if(uci_option.name=="Move Overhead")
		{
			std::istringstream value_stream{uci_option.name};
			if(const std::chrono::milliseconds overhead{Uci_handler::read_time(value_stream)}; overhead>=std::chrono::milliseconds{0} && overhead<=max_move_overhead)
				options.move_overhead=overhead;
			else
				io.output("In setoption name 'Move Overhead': value out of range");
		}
		else
			io.output("Option not found");
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
