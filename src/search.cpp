#include "Chess_data.h"
#include "Constants.h"
#include "Move_generator.h"
#include "Pieces.h"
#include "search.h"
#include "Stdio.h"
#include "Time_manager.h"

#include <algorithm>
#include <array>
#include <format>
#include <limits>

namespace engine
{

	namespace
	{
		[[nodiscard]] Search_result_type compute_type(const int alpha, const int beta, const int score) noexcept
		{
			assert(alpha<beta);

			if(score<=alpha)
				return Search_result_type::upper_bound;
			else if(score>=beta)
				return Search_result_type::lower_bound;
			else
				return Search_result_type::exact;
		}

		[[nodiscard]] bool is_threefold_repetition(const State& state) noexcept
		{
			return std::ranges::count(state.repetition_history | std::views::reverse | std::views::take(state.half_move_clock), state.zobrist_hash)>=3;
		}

		struct Search_context
		{
			State& state;
			unsigned& nodes, &extended_depth;

			const std::atomic<bool>& should_stop_searching;
			const Search_options& search_options;
			const Time_manager& time_manager;
		};

		[[nodiscard]] int quiescence_search(const Search_context& context
										  , unsigned current_extended_depth
										  , int alpha
										  , int beta)
		{
			++context.nodes;
			context.extended_depth=std::max(context.extended_depth,current_extended_depth);

			if(context.should_stop_searching)
				throw search_stopped{};
			if(!context.search_options.depth && context.time_manager.used_time()>context.time_manager.maximum())
				throw timeout{};

			const int stand_pat{context.state.evaluate()};
			int best_score=stand_pat;
			if(stand_pat>=beta || context.extended_depth > 19)
				return stand_pat;
			if(alpha<stand_pat)
				alpha=stand_pat;

			for(const auto& move : generate_moves<Moves_type::noisy>(context.state))
			{
				constexpr int safety_margin{chess_data::piece_values[Piece::pawn]*2};
				if(std::optional<Piece> piece_to_capture{context.state.piece_at(move.destination_square(), other_side(context.state.side_to_move))}; piece_to_capture && stand_pat+chess_data::piece_values[piece_to_capture.value()]+safety_margin<=alpha)
					continue;

				context.state.make(move);
				const int score{-quiescence_search(context, current_extended_depth+1, -beta, -alpha)};
				context.state.unmove();

				if(score>=beta)
					return score;
				if(score>best_score)
					best_score=score;
				if(score>alpha)
					alpha=score;
			}
			return best_score;
		}

		class Killer_move_storage
		{
			public:

			void insert_and_overwrite(const Move& move) noexcept
			{
				killer_moves[older_index]=move;
				++older_index%=2; // (+1)%2 :)
			}

			[[nodiscard]] bool contains(const Move& move) const noexcept
			{
				return killer_moves.front()==move || killer_moves.back()==move;
			}

			private:

			std::size_t older_index{0};
			std::array<Move,2> killer_moves{};
		};

		struct Nega_max_context
		{
			const Search_context& search_context;
			const Fixed_capacity_vector<Move, 256>& principal_variation;
			Transposition_table& transposition_table;

			std::vector<Killer_move_storage> killer_moves{max_depth};
			Fixed_capacity_vector<Move, 256> current_pv{};
		};

		[[nodiscard]] bool most_valuable_vicitim_least_valuable_attacker(const Nega_max_context& context, const Move& lhs, const Move& rhs) noexcept
		{
			const auto lhs_victim_value = std::to_underlying(context.search_context.state.piece_at(lhs.destination_square(), other_side(context.search_context.state.side_to_move)).value());
			const auto rhs_victim_value = std::to_underlying(context.search_context.state.piece_at(rhs.destination_square(), other_side(context.search_context.state.side_to_move)).value());
			if(lhs_victim_value < rhs_victim_value)
				return false;
			if(lhs_victim_value > rhs_victim_value)
				return true;

			const auto lhs_attacker_value = std::to_underlying(context.search_context.state.piece_at(lhs.from_square(), context.search_context.state.side_to_move).value());
			const auto rhs_attacker_value = std::to_underlying(context.search_context.state.piece_at(rhs.from_square(), context.search_context.state.side_to_move).value());
			if(lhs_attacker_value < rhs_attacker_value)
				return true;
			return false;
		};

		[[nodiscard]] bool heuristic_less(const Nega_max_context& context, const auto& cache_result, const int depth, const int remaining_depth, const Move& lhs, const Move& rhs) noexcept
		{
			if(cache_result)
			{
				const auto cached_move=cache_result->best_move;
				const bool lhs_is_cached_move=lhs == cached_move,
				rhs_is_cached_move=rhs == cached_move;
				if(lhs_is_cached_move)
					return true;
				if(rhs_is_cached_move)
					return false;
			}

			if(const auto pv_index=depth-remaining_depth; pv_index<context.principal_variation.size())
			{
				const auto pv_move=context.principal_variation.at(pv_index);
				const bool lhs_is_pv_move=lhs == pv_move,
				rhs_is_pv_move=rhs == pv_move;
				if(lhs_is_pv_move)
					return true;
				if(rhs_is_pv_move)
					return false;
			}

			const bool lhs_is_capture = !is_free(lhs.destination_square(), context.search_context.state.sides[other_side(context.search_context.state.side_to_move)].occupied_squares()),
					   rhs_is_capture = !is_free(rhs.destination_square(), context.search_context.state.sides[other_side(context.search_context.state.side_to_move)].occupied_squares());
			if(lhs_is_capture && rhs_is_capture)
				return most_valuable_vicitim_least_valuable_attacker(context, lhs, rhs);
			if(lhs_is_capture)
				return true;
			if(rhs_is_capture)
				return false;

			// non capture heuristics
			const bool lhs_is_killer{context.killer_moves[remaining_depth].contains(lhs)},
					   rhs_is_killer{context.killer_moves[remaining_depth].contains(rhs)};
			if(lhs_is_killer && rhs_is_killer)
				return false;
			if(lhs_is_killer)
				return true;
			if(rhs_is_killer)
				return false;

			return false;
		}

		[[nodiscard]] int nega_scout(Nega_max_context& context
				   , const unsigned remaining_depth
				   , const unsigned depth
				   , unsigned current_extended_depth
				   , unsigned number_of_checks_in_current_line
				   , int alpha = -std::numeric_limits<int>::max()
				   , int beta = std::numeric_limits<int>::max())
		{
			++context.search_context.nodes;

			context.current_pv.clear();

			if(is_threefold_repetition(context.search_context.state))
				return 0;
			if(!context.search_context.search_options.depth && context.search_context.time_manager.used_time()>context.search_context.time_manager.maximum())
				throw timeout{};

			auto all_legal_moves = generate_moves<Moves_type::legal>(context.search_context.state);
			if(remaining_depth<=0 && !all_legal_moves.empty())
				return quiescence_search(context.search_context, current_extended_depth, alpha, beta);

			const int original_alpha{alpha}, original_beta{beta};
			const bool is_null_window{std::abs(original_alpha-original_beta)<=1};
			const auto cache_result{context.transposition_table[context.search_context.state.zobrist_hash]};
			if(cache_result && cache_result->remaining_depth>=remaining_depth)
			{
				if(cache_result->search_result_type == Search_result_type::exact)
				{
					context.current_pv.push_back(cache_result->best_move);
					return cache_result->eval;
				}
				if(cache_result->search_result_type==Search_result_type::lower_bound)
					alpha=std::max(alpha, cache_result->eval);
				if(cache_result->search_result_type==Search_result_type::upper_bound)
					beta=std::min(beta, cache_result->eval);
				if(alpha>=beta)
					return cache_result->eval;
			}

			const auto sort_move_strength_descending{[&](const Move& lhs, const Move& rhs){ return heuristic_less(context, cache_result, depth, remaining_depth, lhs, rhs); }};
			std::ranges::sort(all_legal_moves, sort_move_strength_descending);

			// If all legal moves lead to checkmate for side to move, this would leave best_move uninitialised
			Move best_move{all_legal_moves.front()};
			int best_score{-std::numeric_limits<int>::max()};
			Fixed_capacity_vector<Move, 256> best_child_pv{};
			bool raised_alpha{false};
			const bool in_check{context.search_context.state.in_check()};
			for(std::size_t move_index{0}; move_index<all_legal_moves.size(); ++move_index)
			{
				const Move& move{all_legal_moves[move_index]};

				if(!context.search_context.search_options.depth && context.search_context.time_manager.used_time()>context.search_context.time_manager.maximum())
					throw timeout{};

				unsigned reduction{1};
				if(move_index>2 && depth>=3)
				{
					if(move_index<=6)
						++reduction;
					else
						reduction+=remaining_depth/3;

					if(reduction>remaining_depth)
						reduction=remaining_depth;
				}
				if(in_check && number_of_checks_in_current_line <= 3)
				{
					--reduction;
					++number_of_checks_in_current_line;
				}
				
				context.search_context.state.make(move);
				struct { int alpha, beta; } null_window{alpha, alpha+1};
				Nega_max_context null_context{context.search_context, {}, context.transposition_table};
				const auto scout_potential_improvement = [&](const unsigned reduction) -> bool
				{
					int null_window_search_score{-nega_scout(null_context, remaining_depth-reduction, depth, current_extended_depth-(reduction-1), number_of_checks_in_current_line, -null_window.beta, -null_window.alpha)};
					const Search_result_type null_window_result{compute_type(null_window.alpha, null_window.beta, null_window_search_score)};
					return null_window_result==Search_result_type::lower_bound;
				};

				if(reduction!=1 && !scout_potential_improvement(reduction))
				{
					context.search_context.state.unmove();
					continue;
				}

				if(const unsigned no_reduction{1}; raised_alpha && remaining_depth>2 && !is_null_window && !scout_potential_improvement(no_reduction))
				{
					context.search_context.state.unmove();
					continue;
				}

				const int score{-nega_scout(context, remaining_depth-reduction, depth, current_extended_depth-(reduction-1), number_of_checks_in_current_line, -beta, -alpha)};
				context.search_context.state.unmove();

				if(score>best_score)
				{
					best_score=score;
					best_move=move;
					best_child_pv=context.current_pv;
				}

				if(score>alpha)
				{
					raised_alpha=true;
					alpha=score;
					if(alpha>=beta)
					{
						context.killer_moves[remaining_depth].insert_and_overwrite(move);
						break;
					}
				}
			}

			if(all_legal_moves.empty())
			{
				if(context.search_context.state.is_square_attacked(context.search_context.state.sides[context.search_context.state.side_to_move].pieces[Piece::king].lsb_square()))
					return -std::numeric_limits<int>::max()+10000;
				else
					return 0;
			}

			best_score=std::clamp(best_score, original_alpha, original_beta);

			context.current_pv.clear();
			context.current_pv.push_back(best_move);
			context.current_pv.insert(context.current_pv.end(), best_child_pv.begin(), best_child_pv.end());

			if(!is_null_window)
			{
				context.transposition_table.insert(Transposition_data
				{
					.remaining_depth=remaining_depth,
					.eval=best_score,
					.zobrist_hash=context.search_context.state.zobrist_hash,
					.search_result_type=compute_type(original_alpha, original_beta, best_score),
					.best_move=best_move
				});
			}

			return best_score;
		};

		void output_info(const int& eval, const auto& nodes, const auto& current_depth, const auto& extended_depth, const auto& principal_variation, const Stdio& io, const int thread_id) noexcept
		{
			const auto output = [&](std::string_view info)
			{
				bool first{true};
				std::ostringstream pv{""};
				for(const auto& move : principal_variation)
				{
					if(!first) pv<<' ';
					first=false;
					pv<<move;
				}
				io.output(info, pv.str());
			};
			if(std::abs(eval)!=std::numeric_limits<int>::max())
				output(std::format("info [Thread {}] score cp {} nodes {} depth {} seldepth {} pv ", thread_id, eval/16, nodes, current_depth, current_depth+extended_depth));
			else
				output(std::format("info [Thread {}] nodes {} depth {} seldepth {} mate ", thread_id, nodes, current_depth, current_depth+extended_depth));
		};
	}

	std::expected<Search_results, search_stopped> iterative_deepening(const std::atomic<bool>& should_stop_searching
														 , const Search_options& search_options
														 , State state
														 , Transposition_table& transposition_table
														 , const int thread_id) noexcept
	{
		static Stdio io;
		Time_manager time_manager(search_options.time[state.side_to_move], search_options.movetime, search_options.increment[state.side_to_move], search_options.move_overhead, search_options.movestogo, state.half_move_clock);
		Fixed_capacity_vector<Move, 256> principal_variation;

		int score{state.evaluate()};
		unsigned nodes{0}, extended_depth{0};
		Nega_max_context nega_max_context{Search_context{state, nodes, extended_depth, should_stop_searching, search_options, time_manager}, principal_variation, transposition_table};

		unsigned current_depth{1};
		for(; search_options.depth? current_depth <= *search_options.depth : current_depth<=max_depth && time_manager.used_time()<time_manager.optimum(); ++current_depth)
		{
			extended_depth=nodes=0;
			nega_max_context.current_pv.clear();
			try
			{
				constexpr int half_initial_window_size{chess_data::piece_values[Piece::pawn]/4};
				int alpha{score-half_initial_window_size}, beta{score+half_initial_window_size};


				// oh dear!!
				if(alpha>=beta) alpha=-std::numeric_limits<int>::max(), beta=std::numeric_limits<int>::max();


				Search_result_type last_search_result_type;
				do
				{
					score=nega_scout(nega_max_context, current_depth, current_depth, 0, 0, alpha, beta);

					last_search_result_type=compute_type(alpha, beta, score);
					if(last_search_result_type==Search_result_type::lower_bound)
						beta=std::numeric_limits<int>::max();
					if(last_search_result_type==Search_result_type::upper_bound)
						alpha=-std::numeric_limits<int>::max();
				} while(last_search_result_type!=Search_result_type::exact);

				principal_variation=nega_max_context.current_pv;
				output_info(score, nodes, current_depth, extended_depth, principal_variation, io, thread_id);
			}
			catch(const timeout&)
			{

			}
			catch(const search_stopped&)
			{
				return std::unexpected{search_stopped{}};
			}
		}

		return Search_results {
			.nodes=nodes,
			.score=score,
			.seldepth=current_depth+extended_depth,
			.pv=principal_variation,
		};
	}
} // namespace engine
