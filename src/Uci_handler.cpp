#ifndef Uci_handler_impl_h_INCLUDED
#define Uci_handler_impl_h_INCLUDED

#include "Constants.h"
#include "Move_generator.h"
#include "search.h"
#include "State.h"
#include "Uci_handler.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace uci
{
	Uci_handler::Uci_handler()
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

	std::chrono::milliseconds Uci_handler::read_time(std::istream& is) noexcept
	{
		unsigned count;
		is>>count;
		return std::chrono::milliseconds{count};
	}

	void Uci_handler::isready_handler() noexcept
	{
		io.output("readyok");
	}

	void Uci_handler::ucinewgame_handler() noexcept
	{
		return;
	}

	void Uci_handler::position_handler(const Input_state& input_state) noexcept
	{
		push_task([this, input_state](std::atomic<bool>&)
		{
			engine.clear_tt();
			engine::State state{input_state.fen};
			for(const auto& move : input_state.continuation)
				state.make(move);
			engine.set_state(state);
		});
	}

	void Uci_handler::go_handler(const Go_options& go_options) noexcept
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
			const auto search_results=engine.generate_best_move(stop_token, search_options, true);
			if(search_results.has_value())
				io.output("bestmove ", search_results->pv.front());
		});
	}

	void Uci_handler::uci_handler() noexcept
	{
		io.output(std::format("id: {}", engine::name));
		io.output(std::format("author: {}\n", engine::author));
		io.output("option name Hash type spin default 16 min 1 max 33554432");
		io.output(std::format("option name Threads type spin default {} min 1 max 1024", engine::default_threads));
		io.output("option name Move Overhead type spin default 10 min 0 max 5000");
		io.output("uciok");
	}

	void Uci_handler::explore_handler() noexcept
	{
		io.output("please enter fen");
		engine::State state{io.input()};
		io.output("please enter your move");
		engine::Move move;
		std::istringstream{io.input()}>>move;
		push_task([sc=state, m=move, this](std::atomic<bool>& stop_token)
		{
			auto state = sc;
			auto move = m;
			const auto legal_moves=engine::generate_moves<engine::Moves_type::legal>(state);
			for(int depth{1}; depth<=9; ++depth)
			{
				engine::Search_options options;
				options.depth=depth;
				std::vector<std::pair<engine::Move, int>> moves_and_score(legal_moves.size());
				for(int i{0}; i<legal_moves.size(); ++i)
				{
					state.make(legal_moves[i]);
					engine.set_state(state);
					if(const auto search_results=engine.generate_best_move(stop_token, options, false); search_results.has_value())
					{
						stop_token=false;
						moves_and_score[i]=std::pair{legal_moves[i], search_results->score};
					}
					else
						return;
					state.unmove();
				}
				std::ranges::sort(moves_and_score, std::greater<>{}, [](const auto& el){ return el.second; });
				const auto it{std::ranges::find(moves_and_score, move, [](const auto& el){ return el.first; })};
				const auto move_number{std::distance(moves_and_score.begin(), it)+1};
				io.output("[depth ", depth, ']', " best move: ", moves_and_score.front().first
							, " with score of ", moves_and_score.front().second/16
							, ", you found move ", move_number, '/', legal_moves.size()
							, " with score of ", it->second/16);
			}
		});
	}

	void Uci_handler::setoption_handler(const Uci_option& uci_option) noexcept
	{
		if(uci_option.name=="Hash")
		{
			if(int value=std::stoi(uci_option.value); value>0 && static_cast<unsigned>(value)<max_table_size)
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
			std::istringstream value_stream{uci_option.value};
			if(const std::chrono::milliseconds overhead{Uci_handler::read_time(value_stream)}; overhead>=std::chrono::milliseconds{0} && overhead<=max_move_overhead)
				options.move_overhead=overhead, io.output(overhead);
			else
				io.output("In setoption name 'Move Overhead': value out of range");
		}
		else
			io.output("Option not found");
	}

	void Uci_handler::stop_handler() noexcept
	{
		should_stop_work=true;
	}

	void Uci_handler::start_listening() noexcept
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
