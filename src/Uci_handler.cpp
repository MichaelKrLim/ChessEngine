#include "Constants.h"
#include "Engine.h"
#include "State.h"
#include "Uci_handler.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace uci;

namespace
{
	engine::State state{};

	bool debug{false};

	struct Input_state
	{
		std::string fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
		std::vector<engine::Move> continuation;
	};

	std::istream& operator>>(std::istream& is, Input_state& input_state)
	{
		std::string command;
		is >> command;
		if(command == "startpos")
			input_state.fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		else if(std::string current_data; command == "fen")
		{
			is >> current_data;
			input_state.fen = current_data;
			// side to move
			is >> current_data;
			input_state.fen += ' '+current_data;
			// castling rights
			is >> current_data;
			input_state.fen += ' '+current_data;
			// en passant
			is >> current_data;
			input_state.fen += ' '+current_data;
			// halfmove clock
			is >> current_data;
			input_state.fen += ' '+current_data;
			// fullmove clock
			is >> current_data;
			input_state.fen += ' '+current_data;
		}
		else
			throw std::invalid_argument{"in reading an Input_state, invalid command"};

		is >> command;
		if(command == "moves")
		{
			for(engine::Move move; is>>move;)
				input_state.continuation.push_back(move);
		}
		return is;
	}

	std::chrono::milliseconds read_time(std::istream& is)
	{
		unsigned count;
		is>>count;
		return std::chrono::milliseconds{count};
	}

	std::istream& operator>>(std::istream& is, Search_options& search_options)
	{
		std::string option;
		while(is>>option)
		{
			if(option == "depth")
			{
				unsigned depth;
				is>>depth;
				search_options.depth = depth;
			}
			else if(option == "wtime")
				search_options.time[engine::Side::white] = read_time(is);
			else if(option == "movetime")
				search_options.movetime = read_time(is);
			else if(option == "winc")
				search_options.increment[engine::Side::white] = read_time(is);
			else if(option == "btime")
				search_options.time[engine::Side::black] = read_time(is);
			else if(option == "binc")
				search_options.increment[engine::Side::black] = read_time(is);
			else if(option == "movestogo")
				is>>search_options.movestogo;
			else if(option == "tt" || option == "hash")
			{
				is>>search_options.transposition_table_size;
				if(option == "hash")
					search_options.transposition_table_size*=1024;
			}
			else
				throw std::invalid_argument{"Command not found"};
		}
		return is;
	};

	void isready_handler() noexcept
	{
		std::cout << "readyok\n";
	}

	void debug_handler(std::string option) noexcept
	{
		if(option == "on")
			debug = true;
		else if(option == "off")
			debug = false;
		else
			std::cout << "debug must be on or off";
	}

	void ucinewgame_handler() noexcept
	{
		state = engine::State{};
	}

	void position_handler(const Input_state& input_state) noexcept
	{
		state = engine::State{input_state.fen};
		for(const auto& move : input_state.continuation)
			state.make(move);
	}

	void go_handler(const Search_options& search_options) noexcept
	{
		const std::optional<engine::Move> best_move = engine::generate_best_move(state, search_options);
		if(best_move)
			std::cout << "bestmove " << best_move.value() << "\n";
		else
			std::cout << "no moves left\n";
	}

	void uci_handler() noexcept
	{
		std::cout << "id: " << engine::name << "\n" 
				  << "author: " << engine::author << "\n"
				  << "uciok\n";
	}

	template <auto>
	struct call_helper;

	template <typename R, typename... ARGS_PACK_T, R (*handler_function)(ARGS_PACK_T...) noexcept>
	struct call_helper<handler_function>
	{
		static void call(std::istringstream& args_stream)
		{
			[&args_stream] <std::size_t... INDEXES_PACK> (std::index_sequence<INDEXES_PACK...>)
			{
				[[maybe_unused]] auto args = std::tuple<std::decay_t<ARGS_PACK_T>...>{};
				((args_stream >> std::get<INDEXES_PACK>(args)), ...);
				(*handler_function)(std::move(std::get<INDEXES_PACK>(args))...);
			}(std::index_sequence_for<ARGS_PACK_T...>{});
		}
	};

	template <auto v>
	constexpr auto call_handler_v = call_helper<v>::call;

	using handler_function_map_t = std::unordered_map<std::string_view, void(*)(std::istringstream&)>;
	const handler_function_map_t to_handler_function
	{
		{"go", call_handler_v<&go_handler>}, {"uci", call_handler_v<&uci_handler>}, {"position", call_handler_v<&position_handler>},
		{"ucinewgame", call_handler_v<&ucinewgame_handler>}, {"isready", call_handler_v<&isready_handler>}, {"debug", call_handler_v<&debug_handler>}
	};
}

namespace uci
{
	void start_listening() noexcept
	{
		for(std::string line, command; command != "quit";)
		{
			std::getline(std::cin, line);
			std::istringstream iss{line};
			iss >> command;
			if(const auto it{to_handler_function.find(command)}; it != to_handler_function.end())
				it->second(iss);
			else
			{
				std::cout << "Command: " << command << " not found";
				return;
			}
		}
	}
}
