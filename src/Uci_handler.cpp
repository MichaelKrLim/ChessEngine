#include "Constants.h"
#include "State.h"
#include "Uci_handler.h"

#include <array>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>

namespace
{
	engine::State state{};
	
	void isready_handler() noexcept
	{
		std::cout << "readyok\n";
	}

	void ucinewgame_handler() noexcept
	{
		state = engine::State{};
	}

	void position_handler(const std::string& line) noexcept
	{
		auto commands_string = line | std::ranges::views::split(' ');
		std::vector<std::string_view> commands = commands_string | std::ranges::views::transform([](auto&& seg)
		{
			return std::string_view{std::ranges::begin(seg), std::ranges::end(seg)};
		}) | std::ranges::to<std::vector>();

		assert(commands[0] == "position" && !commands.empty());
		int move_list_index{0};
		if(commands[1] == "startpos")
		{
			move_list_index = 2;
			state = engine::State{engine::starting_fen};
		}
		if(commands[1] == "fen")
		{
			move_list_index = 3;
			state = engine::State{commands[2]};
		}
		for(int i{move_list_index}; i<commands.size(); ++i)
		{
			const auto move = commands[i];
			state.make(engine::Move{engine::algebraic_to_position(move.substr(0, 2)), engine::algebraic_to_position(move.substr(2, 2))});
		}
	}

	void go_handler() noexcept
	{

	}

	void uci_handler() noexcept
	{
		std::cout << "id: " << engine::name << "\n" 
				  << "author: " << engine::author << "\n"
				  << "uciok\n";
	}

	using handler_function_map_t = std::unordered_map<std::string, std::function<void()>>;
	handler_function_map_t to_handler_function = []()
	{
		handler_function_map_t handler_functions;
		struct Handler_command_pair
		{
			std::string command;
			const std::function<void()> handler_function;
		};
		std::array<Handler_command_pair, 4> handler_command_pairs
		{{
			{"go", go_handler}, {"uci", uci_handler},
			{"isready", isready_handler}, {"ucinewgame", ucinewgame_handler},
		}};
		for(const Handler_command_pair& handler_command_pair : handler_command_pairs)
		{
			handler_functions[handler_command_pair.command] = handler_command_pair.handler_function;
		}
		return handler_functions;
	}();
}

namespace uci
{
	void start_listening() noexcept
	{
		for(std::string line{}; line != "quit"; std::getline(std::cin, line))
		{
			const auto trim_whitespace = [](std::string& to_trim)
			{
				const auto leading_whitespace = std::find_if_not(to_trim.begin(), to_trim.end(), ::isspace);
				const auto trailing_whitespace = std::find_if_not(to_trim.rbegin(), to_trim.rend(), ::isspace).base();
				to_trim = (leading_whitespace < trailing_whitespace ? std::string{leading_whitespace, trailing_whitespace} : std::string{""});
			};
			trim_whitespace(line);
			if(auto it = to_handler_function.find(line); it != to_handler_function.end())
			{
				(to_handler_function[line])();
			}
			else
				position_handler(line);
		}
	}
}
