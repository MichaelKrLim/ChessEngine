#include "Constants.h"
#include "Move.h"
#include "State.h"
#include "Uci_handler.h"

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <print>
#include <sstream>
#include <string>

using namespace uci;

Uci_handler::Uci_handler()
{
	worker_thread = std::jthread{[this](std::stop_token stop_token)
	{
		std::unique_lock queue_lock(queue_mtx);
		for(;;)
		{
			condition_variable.wait(queue_lock, [&](){ return !task_queue.empty(); });
			const auto task{task_queue.front()};
			queue_lock.unlock();
			task(stop_token);
			queue_lock.lock();
			task_queue.pop(); // after call for shared pointer
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
	std::cout << "readyok" << std::endl;
}

void Uci_handler::ucinewgame_handler() noexcept
{
	return;
}

void Uci_handler::position_handler(const Input_state& input_state) noexcept
{
	engine.clear_tt();
	engine::State state = engine::State{input_state.fen};
	for(const auto& move : input_state.continuation)
		state.make(move);
	engine.set_state(state);
}

void Uci_handler::go_handler(const engine::Search_options& search_options) noexcept
{
	task_queue.push([&](std::stop_token stop_token)
	{
		const auto search_results = engine.generate_best_move(stop_token, search_options);
		if(!(search_results.error()==engine::Engine::search_stopped{}))
			std::cout << "bestmove " << search_results->pv.front() << "\n";
	});
}

void Uci_handler::uci_handler() noexcept
{
	std::println("id: {}", engine::name);
	std::println("author: {}\n", engine::author);
	std::println("option name Hash type spin default 16 min 1 max 33554432");
	std::println("option name Threads type spin default 1 min 1 max 1");
	std::println("option name Move Overhead type spin default 10 min 0 max 5000");
	std::println("uciok");
}

void Uci_handler::setoption_handler(const engine::Engine_options& new_engine_options) noexcept
{
	engine.set_options(new_engine_options);
}

void Uci_handler::stop_handler() noexcept
{
	worker_thread.request_stop();
}

void Uci_handler::start_listening() noexcept
{
	for(std::string line, command; std::getline(std::cin, line) && line != "quit";)
	{
		std::istringstream iss{line};
		iss>>command;
		if(const auto it{to_handler_function.find(command)}; it != to_handler_function.end())
		{
			if(command == "position" || command == "go")
			{
				std::lock_guard lock(queue_mtx);
				task_queue.push([this, iss=std::make_shared<std::istringstream>(std::move(iss)), helper=it->second](std::stop_token) mutable { helper(*this, *iss); });
				condition_variable.notify_one();
			}
			else
				it->second(*this, iss);
		}
		else
			std::println("command: {} not found", command);
	}
	worker_thread.request_stop();
}
