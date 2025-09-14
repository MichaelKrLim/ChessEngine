#ifndef Uci_handler_h_INCLUDED
#define Uci_handler_h_INCLUDED

#include "Constants.h"
#include "Engine.h"

#include <chrono>
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

	struct Go_options
	{
		std::optional<unsigned> depth{std::nullopt};
		unsigned movestogo{0};
		engine::Side_map<std::optional<std::chrono::milliseconds>> time{std::nullopt, std::nullopt};
		engine::Side_map<std::chrono::milliseconds> increment{std::chrono::milliseconds{0}, std::chrono::milliseconds{0}};
		std::optional<std::chrono::milliseconds> movetime{std::nullopt};
	};

	struct Uci_options
	{
		int threads{engine::default_threads};
		std::chrono::milliseconds move_overhead{engine::default_move_overhead};
	};

	struct Uci_option
	{
		std::string name;
		std::string value;
	};

	inline std::istream& operator>>(std::istream& is, Uci_option& engine_options);
	inline std::istream& operator>>(std::istream& is, Go_options& engine_options);
	inline std::istream& operator>>(std::istream& is, Input_state& input_state);

	template <typename Io>
	class Uci_handler
	{
		public:

		void start_listening() noexcept;
		Uci_handler();
		static std::chrono::milliseconds read_time(std::istream& is) noexcept;

		private:

		using Task=std::move_only_function<void(std::atomic<bool>&) const>;

		template <typename Task>
		inline void push_task(Task&& task) noexcept
		{
			std::lock_guard lock(queue_mtx);
			task_queue.push(std::forward<Task>(task));
			condition_variable.notify_one();
		}

		inline std::optional<Task> pop_task(std::stop_token stop_token) noexcept
		{
			std::unique_lock queue_lock(queue_mtx);

			condition_variable.wait(queue_lock, [&](){ return stop_token.stop_requested() || !task_queue.empty(); });

			if(stop_token.stop_requested())
				return std::nullopt;

			auto task{std::move(task_queue.front())};
			task_queue.pop();
			return task;
		};

		void isready_handler() noexcept;
		void ucinewgame_handler() noexcept;
		void position_handler(const Input_state& input_state) noexcept;
		void go_handler(const Go_options& search_options) noexcept;
		void uci_handler() noexcept;
		void setoption_handler(const Uci_option& uci_option) noexcept;
		void stop_handler() noexcept;

		engine::Engine<Io> engine;
		std::jthread worker_thread;
		std::queue<Task> task_queue;
		std::mutex queue_mtx;
		std::condition_variable condition_variable;
		std::atomic<bool> should_stop_work;
		Uci_options options{};
		Io io;

		friend inline std::istream& operator>>(std::istream& is, Uci_option& engine_options)
		{
			std::string ignore;
			is>>ignore;
			for(std::string word; is>>word && word!="value";)
			{
				if(!engine_options.name.empty())
					engine_options.name+=' ';
				engine_options.name+=word;
			}
			if(!is)
				throw std::invalid_argument{"Option not found"};
			return is>>engine_options.value;
		}

		friend inline std::istream& operator>>(std::istream& is, Input_state& input_state)
		{
			std::string command;
			is>>command;
			if(command=="startpos")
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

		friend inline std::istream& operator>>(std::istream& is, Go_options& go_options)
		{
			std::string option;
			while(is>>option)
			{
				if(option=="depth")
				{
					unsigned depth;
					is>>depth;
					go_options.depth = depth;
				}
				else if(option=="wtime")
					go_options.time[engine::Side::white]=Uci_handler::read_time(is);
				else if(option=="movetime")
					go_options.movetime=Uci_handler::read_time(is);
				else if(option=="winc")
					go_options.increment[engine::Side::white]=Uci_handler::read_time(is);
				else if(option=="btime")
					go_options.time[engine::Side::black]=Uci_handler::read_time(is);
				else if(option=="binc")
					go_options.increment[engine::Side::black]=Uci_handler::read_time(is);
				else if(option=="movestogo")
					is>>go_options.movestogo;
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
			{"setoption",  call_handler_v<&Uci_handler::setoption_handler>}
		};
	};
} // namespace uci

#include "Uci_handler_impl.h"

#endif // Uci_handler_h_INCLUDED
