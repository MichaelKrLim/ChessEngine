#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Bitboard.h"
#include "Chess_data.h"
#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move.h"
#include "Move_generator.h"
#include "Pieces.h"
#include "State.h"
#include "Transposition_table.h"
#include "Uci_handler.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <limits>

namespace engine
{
	inline Move generate_best_move(State state, const uci::Search_options& search_options)
	{
		const auto stop_searching = [&search_options, start_time=std::chrono::high_resolution_clock::now(), side=state.side_to_move](const std::optional<unsigned> current_depth = std::nullopt)
		{
			if(search_options.depth && current_depth && current_depth.value() > search_options.depth.value())
				return true;
			else
			{
				const auto used_time = std::chrono::high_resolution_clock::now()-start_time+std::chrono::milliseconds{100}; // buffer
				if((search_options.movetime && used_time > search_options.movetime.value()) || (search_options.time[side] && used_time > (search_options.time[side].value()+22*search_options.increment[side])/22))
					return true;
			}
			return false;
		};

		Transposition_table transposition_table(search_options.hash);

		static auto compute_type = [](const double alpha, const double beta, const double score)
		{
			if(score<=alpha)      return Search_result_type::upper_bound;
			else if(score>=beta)  return Search_result_type::lower_bound;
			else                  return Search_result_type::exact;
		};

		enum class timeout {};

		const auto quiescence_search = [&state, &stop_searching](this auto&& rec, double alpha, double beta, unsigned& extended_depth, unsigned& nodes, const unsigned additional_depth=1) -> double
		{
			++nodes;
			extended_depth = std::max(extended_depth, additional_depth);

			if(state.repetition_history[state.zobrist_hash] >= 3)
				return 0.0;
			else if(stop_searching())
				throw timeout{};

			const double& stand_pat = state.evaluation*(state.side_to_move==Side::white? 1:-1);
			double best_score = stand_pat;
			if(stand_pat >= beta)
				return stand_pat;
			if(alpha < stand_pat)
				alpha = stand_pat;

			for(const auto& move : generate_moves<Moves_type::noisy>(state))
			{
				if(std::optional<Piece> piece_to_capture=state.piece_at(move.destination_square(), other_side(state.side_to_move)); piece_to_capture && stand_pat+chess_data::piece_values[Side::white][piece_to_capture.value()]<=alpha)
					continue;
				state.make(move);
				const double score = -rec(-beta, -alpha, extended_depth, nodes, additional_depth+1);
				state.unmove();
				if(score >= beta)
					return score;
				if(score > best_score)
					best_score = score;
				if(score > alpha)
					alpha = score;
			}
			return best_score;
		};

		Fixed_capacity_vector<Move, 256> principal_variation;
		const auto nega_max = [&](this auto&& rec, const unsigned remaining_depth, unsigned& extended_depth, unsigned& nodes, const unsigned& depth, Fixed_capacity_vector<Move, 256>& pv, double alpha = -std::numeric_limits<double>::infinity(), double beta = std::numeric_limits<double>::infinity())
		{
			++nodes;
			pv.clear();
			if(state.repetition_history[state.zobrist_hash] >= 3)
				return 0.0;
			else if(remaining_depth <= 0)
				return quiescence_search(alpha, beta, extended_depth, nodes);
			else if(stop_searching())
				throw timeout{};

			const double original_alpha = alpha, original_beta = beta;
			const auto cache_result = transposition_table[state.zobrist_hash];
			if(cache_result && cache_result->remaining_depth >= remaining_depth)
			{
				if(cache_result->search_result_type == Search_result_type::exact)
				{
					pv.push_back(cache_result->best_move);
					return cache_result->eval;
				}
				if(cache_result->search_result_type == Search_result_type::lower_bound)
					alpha = std::max(alpha, cache_result->eval);
				if(cache_result->search_result_type == Search_result_type::upper_bound)
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

			auto all_legal_moves = generate_moves<Moves_type::legal>(state);
			std::ranges::sort(all_legal_moves, [&](const Move& lhs, const Move& rhs)
			{
				if(cache_result)
				{
					const auto cached_move = cache_result->best_move;
					const bool lhs_is_cached_move = lhs == cached_move,
					rhs_is_cached_move = rhs == cached_move;
					if(lhs_is_cached_move)
						return true;
					if(rhs_is_cached_move)
						return false;
				}
				if(const auto pv_index = depth - remaining_depth; pv_index < principal_variation.size())
				{
					const auto pv_move = principal_variation[pv_index];
					const bool lhs_is_pv_move = lhs == pv_move,
					rhs_is_pv_move = rhs == pv_move;
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
				return false;
			});

			Move best_move;
			double best_score{-std::numeric_limits<double>::infinity()};
			bool found_move{false};
			class
			{
				public:

				void invert_best_pv_index()
				{
					best_index+=1;
					best_index%=2;
				}
				auto& best_child_pv()
				{
					return child_pvs[best_index];
				}
				auto& inferior_child_pv()
				{
					return child_pvs[(best_index+1)%2];
				}

				private:

				unsigned best_index{0};
				std::array<Fixed_capacity_vector<Move, 256>, 2> child_pvs{};
			} child_pvs;
			bool raised_alpha{false};
			for(std::size_t move_index{0}; move_index<all_legal_moves.size(); ++move_index)
			{
				const auto& move = all_legal_moves[move_index];

				if(stop_searching())
					throw timeout{};

				unsigned reduction{1};
				if(move_index>1)
				{
					reduction+=move_index<=6? 1:remaining_depth/3;
					if(reduction>remaining_depth)
						reduction=remaining_depth;
				}

				state.make(move);

				struct { double alpha, beta; } null_window{alpha, alpha+1};
				const auto scout_potential_improvement = [&](const unsigned reduction) -> bool
				{
					double null_window_search_score{-rec(remaining_depth-reduction, extended_depth, nodes, depth, child_pvs.inferior_child_pv(), -null_window.beta, -null_window.alpha)};
					Search_result_type null_window_result{compute_type(null_window.alpha, null_window.beta, null_window_search_score)};
					return null_window_result==Search_result_type::lower_bound;
				};

				if(reduction!=1 && !scout_potential_improvement(reduction))
				{
					state.unmove();
					continue;
				}

				if(const unsigned no_reduction{1}; raised_alpha && remaining_depth>2 && beta-alpha>1 && !scout_potential_improvement(no_reduction))
				{
					state.unmove();
					continue;
				}

				double score=-rec(remaining_depth-1, extended_depth, nodes, depth, child_pvs.inferior_child_pv(), -beta, -alpha);

				state.unmove();
				if(score>best_score)
				{
					found_move=true;
					best_score=score;
					best_move=move;
					child_pvs.invert_best_pv_index();
				}
				if(score>alpha)
				{
					raised_alpha=true;
					alpha=score;
					if(alpha>=beta)
						break;
				}
			}

			if(all_legal_moves.empty())
			{
				if(state.is_square_attacked(state.sides[state.side_to_move].pieces[Piece::king].lsb_square()))
					return -std::numeric_limits<double>::infinity();
				else
					return 0.0;
			}

			if(found_move)
			{
				pv.push_back(best_move);
				pv.insert(pv.end(), child_pvs.best_child_pv().begin(), child_pvs.best_child_pv().end());
			}

			transposition_table.insert(Transposition_data
			{
				.remaining_depth = remaining_depth,
				.eval = best_score,
				.zobrist_hash = state.zobrist_hash,
				.search_result_type = compute_type(original_alpha, original_beta, alpha),
				.best_move = best_move
			});

			return best_score;
		};

		const auto output_info = [](const auto& eval, const auto& nodes, const auto& current_depth, const auto& extended_depth, const auto& principal_variation)
		{
			const auto output_pv = [](const Fixed_capacity_vector<Move, 256>& principal_variation)
			{
				bool first{true};
				for(const auto& move : principal_variation)
				{
					if(!first) std::cout<<' ';
					std::cout<<move;
					first=false;
				}
			};
			std::cout << "info"
			          << " score cp " << eval
			          << " nodes " << nodes
			          << " depth " << current_depth
			          << " seldepth " << current_depth+extended_depth
			          << " pv ";
			output_pv(principal_variation);
			std::cout << "\n";
		};

		/*Move best_move; = [&state]()
		{
			struct Move_data { Move move; double eval; };
			const auto scores = generate_moves<Moves_type::legal>(state) | std::views::transform([&state](const Move& move)
			{
				state.make(move);
				const auto eval = evaluate(state);
				state.unmove();
				return Move_data{move, eval};
			});
			const auto it=std::ranges::max_element(scores, {}, &Move_data::eval);
			return (*it).move;
		}();*/
		
		for(unsigned current_depth{1}; !stop_searching(current_depth); ++current_depth)
		{
			principal_variation.clear();
			try
			{
				unsigned extended_depth{0},
				nodes{0};
				Fixed_capacity_vector<Move, 256> current_pv;
				const double eval = nega_max(current_depth, extended_depth, nodes, current_depth, current_pv)*(state.side_to_move==Side::black?-1:1);
				principal_variation = current_pv;
				output_info(eval, nodes, current_depth, extended_depth, principal_variation);
				if(std::abs(eval) == std::numeric_limits<double>::infinity())
					return principal_variation.front();
				principal_variation.clear();
			}
			catch(const timeout&) { break; }
		}
		return principal_variation.front();
	}
}

#endif //Engine_h_INCLUDED
