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
	std::expected<Search_results, search_stopped> nega_max(const std::atomic<bool>& should_stop_searching
														 , const Search_options& search_options
														 , State state
														 , Transposition_table& transposition_table
														 , const int thread_id) noexcept
	{
		static Stdio io;

		Time_manager time_manager(search_options.time[state.side_to_move], search_options.movetime, search_options.increment[state.side_to_move], search_options.move_overhead, search_options.movestogo, state.half_move_clock);

		const static auto compute_type=[](const int alpha, const int beta, const int score)
		{
			assert(alpha<beta);
			if(score<=alpha)      return Search_result_type::upper_bound;
			else if(score>=beta)  return Search_result_type::lower_bound;
			else                  return Search_result_type::exact;
		};

		const auto is_threefold_repetition=[&]()
		{
			return std::ranges::count(state.repetition_history | std::views::reverse | std::views::take(state.half_move_clock), state.zobrist_hash)>=3;
		};

		const auto quiescence_search = [&](this auto&& rec, int alpha, int beta, int& extended_depth, const int& current_extended_depth, unsigned& nodes, const unsigned depth) -> int
		{
			++nodes;
			extended_depth = std::max(extended_depth, current_extended_depth);

			if(should_stop_searching)
				throw search_stopped{};
			if(!search_options.depth && time_manager.used_time()>time_manager.maximum())
				throw timeout{};

			const int& stand_pat=state.evaluate();
			int best_score=stand_pat;
			if(stand_pat>=beta || extended_depth > 19)
				return stand_pat;
			if(alpha<stand_pat)
				alpha=stand_pat;

			for(const auto& move : generate_moves<Moves_type::noisy>(state))
			{
				constexpr int safety_margin{chess_data::piece_values[Piece::pawn]*2};
				if(std::optional<Piece> piece_to_capture=state.piece_at(move.destination_square(), other_side(state.side_to_move)); piece_to_capture && stand_pat+chess_data::piece_values[piece_to_capture.value()]+safety_margin<=alpha)
					continue;
				state.make(move);
				const int score=-rec(-beta, -alpha, extended_depth, current_extended_depth+1, nodes, depth);
				state.unmove();
				if(score>=beta)
					return score;
				if(score>best_score)
					best_score = score;
				if(score>alpha)
					alpha = score;
			}
			return best_score;
		};

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
			std::array<Move, 2> killer_moves{};
		};
		std::vector<Killer_move_storage> killer_moves;
		killer_moves.reserve(max_depth);
		Fixed_capacity_vector<Move, 256> principal_variation;
		const auto nega_max = [&](this auto&& rec
								, const unsigned remaining_depth
								, int& extended_depth
								, int current_extended_depth
								, unsigned number_of_checks_in_current_line
								, unsigned& nodes
								, const unsigned& depth
								, Fixed_capacity_vector<Move, 256>& pv
								, int alpha = -std::numeric_limits<int>::max()
								, int beta = std::numeric_limits<int>::max())
		{
			killer_moves.resize(std::max<std::size_t>(killer_moves.size(), remaining_depth+1));

			++nodes;
			pv.clear();

			if(is_threefold_repetition())
				return 0;
			if(!search_options.depth && time_manager.used_time()>time_manager.maximum())
				throw timeout{};

			auto all_legal_moves = generate_moves<Moves_type::legal>(state);
			if(remaining_depth<=0 && !all_legal_moves.empty())
				return quiescence_search(alpha, beta, extended_depth, current_extended_depth, nodes, depth);

			const int original_alpha{alpha}, original_beta{beta};
			const bool is_null_window{std::abs(original_alpha-original_beta)<=1};
			const auto cache_result=transposition_table[state.zobrist_hash];
			if(cache_result && cache_result->remaining_depth>=remaining_depth)
			{
				if(cache_result->search_result_type == Search_result_type::exact)
				{
					pv.push_back(cache_result->best_move);
					return cache_result->eval;
				}
				if(cache_result->search_result_type==Search_result_type::lower_bound)
					alpha = std::max(alpha, cache_result->eval);
				if(cache_result->search_result_type==Search_result_type::upper_bound)
					beta = std::min(beta, cache_result->eval);
				if(alpha>=beta)
					return cache_result->eval;
			}

			const auto most_valuable_vicitim_least_valuable_attacker = [&](const Move& lhs, const Move& rhs)
			{
				const auto lhs_victim_value = std::to_underlying(state.piece_at(lhs.destination_square(), other_side(state.side_to_move)).value());
				const auto rhs_victim_value = std::to_underlying(state.piece_at(rhs.destination_square(), other_side(state.side_to_move)).value());
				if(lhs_victim_value < rhs_victim_value)
					return false;
				if(lhs_victim_value > rhs_victim_value)
					return true;

				const auto lhs_attacker_value = std::to_underlying(state.piece_at(lhs.from_square(), state.side_to_move).value());
				const auto rhs_attacker_value = std::to_underlying(state.piece_at(rhs.from_square(), state.side_to_move).value());
				if(lhs_attacker_value < rhs_attacker_value)
					return true;
				return false;
			};

			std::ranges::sort(all_legal_moves, [&](const Move& lhs, const Move& rhs)
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

				if(const auto pv_index=depth-remaining_depth; pv_index<principal_variation.size())
				{
					const auto pv_move=principal_variation[pv_index];
					const bool lhs_is_pv_move=lhs == pv_move,
					rhs_is_pv_move=rhs == pv_move;
					if(lhs_is_pv_move)
						return true;
					if(rhs_is_pv_move)
						return false;
				}

				const bool lhs_is_capture = !is_free(lhs.destination_square(), state.sides[other_side(state.side_to_move)].occupied_squares()),
						   rhs_is_capture = !is_free(rhs.destination_square(), state.sides[other_side(state.side_to_move)].occupied_squares());
				if(lhs_is_capture && rhs_is_capture)
					return most_valuable_vicitim_least_valuable_attacker(lhs, rhs);
				if(lhs_is_capture)
					return true;
				if(rhs_is_capture)
					return false;

				// non capture heuristics
				const bool lhs_is_killer{killer_moves[remaining_depth].contains(lhs)},
						   rhs_is_killer{killer_moves[remaining_depth].contains(rhs)};
				if(lhs_is_killer && rhs_is_killer)
					return false;
				if(lhs_is_killer)
					return true;
				if(rhs_is_killer)
					return false;

				return false;
			});

			// If all legal moves lead to checkmate for side to move, this would leave best_move uninitialised
			Move best_move{all_legal_moves.front()};
			int best_score{-std::numeric_limits<int>::max()};
			class
			{
				public:

				void invert_best_pv_index() noexcept
				{
					best_index+=1;
					best_index%=2;
				}

				[[nodiscard]] auto& best_child_pv() noexcept
				{
					return child_pvs[best_index];
				}

				[[nodiscard]] auto& inferior_child_pv() noexcept
				{
					return child_pvs[(best_index+1)%2];
				}

				private:

				std::size_t best_index{0};
				std::array<Fixed_capacity_vector<Move, 256>, 2> child_pvs{};
			} child_pvs;
			bool raised_alpha{false};
			const bool in_check{state.in_check()};
			for(std::size_t move_index{0}; move_index<all_legal_moves.size(); ++move_index)
			{
				const auto& move = all_legal_moves[move_index];

				if(!search_options.depth && time_manager.used_time()>time_manager.maximum())
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
				
				state.make(move);
				struct { int alpha, beta; } null_window{alpha, alpha+1};
				const auto scout_potential_improvement = [&](const unsigned reduction) -> bool
				{
					int null_window_search_score{-rec(remaining_depth-reduction, extended_depth, current_extended_depth+reduction, number_of_checks_in_current_line, nodes, depth, child_pvs.inferior_child_pv(), -null_window.beta, -null_window.alpha)};
					const Search_result_type null_window_result{compute_type(null_window.alpha, null_window.beta, null_window_search_score)};
					return null_window_result==Search_result_type::lower_bound;
				};

				if(reduction!=1 && !scout_potential_improvement(reduction))
				{
					state.unmove();
					continue;
				}

				if(const unsigned no_reduction{1}; raised_alpha && remaining_depth>2 && !is_null_window && !scout_potential_improvement(no_reduction))
				{
					state.unmove();
					continue;
				}

				const int score=-rec(remaining_depth-1, extended_depth, current_extended_depth+reduction, number_of_checks_in_current_line, nodes, depth, child_pvs.inferior_child_pv(), -beta, -alpha);
				state.unmove();

				if(score>best_score)
				{
					best_score=score;
					best_move=move;
					child_pvs.invert_best_pv_index();
				}

				if(score>alpha)
				{
					raised_alpha=true;
					alpha=score;
					if(alpha>=beta)
					{
						killer_moves[remaining_depth].insert_and_overwrite(move);
						break;
					}
				}
			}

			if(all_legal_moves.empty())
			{
				if(state.is_square_attacked(state.sides[state.side_to_move].pieces[Piece::king].lsb_square()))
					return -std::numeric_limits<int>::max()+10000;
				else
					return 0;
			}

			best_score=std::max(best_score,original_alpha);

			pv.push_back(best_move);
			const auto& child_pv{child_pvs.best_child_pv()};
			pv.insert(pv.end(), child_pv.begin(), child_pv.end());

			if(!is_null_window)
			{
				transposition_table.insert(Transposition_data
				{
					.remaining_depth=remaining_depth,
					.eval=best_score,
					.zobrist_hash=state.zobrist_hash,
					.search_result_type=compute_type(original_alpha, original_beta, best_score),
					.best_move=best_move
				});
			}

			return best_score;
		};

		const auto output_info = [&](const int& eval, const auto& nodes, const auto& current_depth, const auto& extended_depth, const auto& principal_variation)
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

		Fixed_capacity_vector<Move, 256> current_pv;
		int extended_depth{0}, score{state.evaluate()};
		unsigned nodes{0}, current_depth{1};
		for(; search_options.depth? current_depth <= *search_options.depth : current_depth<=max_depth && time_manager.used_time()<time_manager.optimum(); ++current_depth)
		{
			extended_depth=nodes=0;
			try
			{
				constexpr double half_initial_window_size{chess_data::piece_values[Piece::pawn]/4.0};
				int alpha=score-half_initial_window_size, beta=score+half_initial_window_size;


				// oh dear!!
				if(alpha>=beta) alpha=-std::numeric_limits<int>::max(), beta=std::numeric_limits<int>::max();


				Search_result_type last_search_result_type;
				do
				{
					score=nega_max(current_depth, extended_depth, 0, 0, nodes, current_depth, current_pv, alpha, beta);
					last_search_result_type=compute_type(alpha, beta, score);

					if(last_search_result_type==Search_result_type::lower_bound)
						beta=std::numeric_limits<int>::max();//+=chess_data::piece_values[Piece::pawn];
					if(last_search_result_type==Search_result_type::upper_bound)
						alpha=-std::numeric_limits<int>::max();//-=chess_data::piece_values[Piece::pawn];
				} while(last_search_result_type!=Search_result_type::exact);

				principal_variation = current_pv;
				output_info(score, nodes, current_depth, extended_depth, principal_variation);
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
			.pv=current_pv,
		};
	}
} // namespace engine
