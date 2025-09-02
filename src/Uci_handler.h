#ifndef Uci_handler_h_INCLUDED
#define Uci_handler_h_INCLUDED

#include "Constants.h"
#include "Engine.h"

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

namespace uci
{
	struct Input_state
	{
		std::string fen{engine::starting_fen};
		std::vector<engine::Move> continuation;
	};

	inline std::istream& operator>>(std::istream& is, engine::Engine_options& engine_options);
	inline std::istream& operator>>(std::istream& is, engine::Search_options& engine_options);
	inline std::istream& operator>>(std::istream& is, Input_state& input_state);

	template <typename Io>
	class Uci_handler
	{
		public:

		void start_listening() noexcept;
		Uci_handler();
		static std::chrono::milliseconds read_time(std::istream& is) noexcept;

		private:

		template <typename Task>
		inline void push_task(Task&& task) noexcept
		{
			std::lock_guard lock(queue_mtx);
			task_queue.push(std::forward<Task>(task));
			condition_variable.notify_one();
		}

		void isready_handler() noexcept;
		void ucinewgame_handler() noexcept;
		void position_handler(const Input_state& input_state) noexcept;
		void go_handler(const engine::Search_options& search_options) noexcept;
		void uci_handler() noexcept;
		void setoption_handler(const engine::Engine_options& new_engine_options) noexcept;
		void stop_handler() noexcept;

		engine::Engine engine;
		std::jthread worker_thread;
		std::queue<std::move_only_function<void(const std::atomic<bool>&) const>> task_queue;
		std::mutex queue_mtx;
		std::condition_variable condition_variable;
		std::atomic<bool> should_stop_work;
		Io io;

		friend inline std::istream& operator>>(std::istream& is, engine::Engine_options& engine_options)
		{
			std::string option;
			is>>option>>option;
			for(std::string word; word!="value"; is>>word)
			{
				if(!word.empty())
					option+=' ';
				option+=word;
			}
			if(unsigned value; option == "Hash" && is>>value && value>0 && value<max_table_size)
				is>>engine_options.hash;
			else if(option == "Threads")
				return is;
			else if(const std::chrono::milliseconds overhead{Uci_handler::read_time(is)}; option == "Move Overhead" && overhead>=std::chrono::milliseconds{0} && overhead<=max_move_overhead)
				engine_options.move_overhead=overhead;
			else
				throw std::invalid_argument{"Option not found"};
			return is;
		}
		
		friend inline std::istream& operator>>(std::istream& is, Input_state& input_state)
		{
			std::string command;
			is>>command;
			if(command == "startpos")
				input_state.fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
			else if(std::string current_data; command == "fen")
			{
				is>>current_data;
				input_state.fen=current_data;
				// side to move
				is>>current_data;
				input_state.fen+=' '+current_data;
				// castling rights
				is>>current_data;
				input_state.fen+=' '+current_data;
				// en passant
				is>>current_data;
				input_state.fen+=' '+current_data;
				// halfmove clock
				is>>current_data;
				input_state.fen+=' '+current_data;
				// fullmove clock
				is>>current_data;
				input_state.fen+=' '+current_data;
			}
			else
				throw std::invalid_argument{"in reading an Input_state, invalid command"};

			is>>command;
			if(command=="moves")
			{
				for(engine::Move move; is>>move;)
					input_state.continuation.push_back(move);
			}
			return is;
		}

		friend inline std::istream& operator>>(std::istream& is, engine::Search_options& search_options)
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
					search_options.time[engine::Side::white]=Uci_handler::read_time(is);
				else if(option == "movetime")
					search_options.movetime=Uci_handler::read_time(is);
				else if(option == "winc")
					search_options.increment[engine::Side::white]=Uci_handler::read_time(is);
				else if(option == "btime")
					search_options.time[engine::Side::black]=Uci_handler::read_time(is);
				else if(option == "binc")
					search_options.increment[engine::Side::black]=Uci_handler::read_time(is);
				else if(option == "movestogo")
					is>>search_options.movestogo;
				else
					throw std::invalid_argument{"Command not found"};
			}
			return is;
		}

		template <auto>
		struct call_helper;

		template <typename... ARGS_PACK_T, void (Uci_handler::*handler_function)(ARGS_PACK_T...) noexcept>
		struct call_helper<handler_function>
		{
			static void call(Uci_handler& uci_handler, std::istringstream& args_stream)
			{
				[&] <std::size_t... INDEXES_PACK> (std::index_sequence<INDEXES_PACK...>)
				{
					[[maybe_unused]] auto args = std::tuple<std::decay_t<ARGS_PACK_T>...>{};
					((args_stream >> std::get<INDEXES_PACK>(args)), ...);
					(uci_handler.*handler_function)(std::move(std::get<INDEXES_PACK>(args))...);
				}(std::index_sequence_for<ARGS_PACK_T...>{});
			}
		};

		template <auto v>
		constexpr static auto call_handler_v = call_helper<v>::call;

		using handler_function_map_t = std::unordered_map<std::string_view, void(*)(Uci_handler&, std::istringstream&)>;

		const handler_function_map_t to_handler_function
		{
			{"go",         call_handler_v<&Uci_handler::go_handler>        },
			{"uci",        call_handler_v<&Uci_handler::uci_handler>       },
			{"position",   call_handler_v<&Uci_handler::position_handler>  },
			{"stop",       call_handler_v<&Uci_handler::stop_handler>      },
			{"ucinewgame", call_handler_v<&Uci_handler::ucinewgame_handler>},
			{"isready",    call_handler_v<&Uci_handler::isready_handler>   },
			{"setoption",  call_handler_v<&Uci_handler::setoption_handler >}
		};
	};
} // namespace uci

#include "Uci_handler_impl.h"

#endif // Uci_handler_h_INCLUDED
