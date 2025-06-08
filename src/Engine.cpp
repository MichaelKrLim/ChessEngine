#include "Bitboard.h"
#include "Constants.h"
#include "Engine.h"
#include "Fixed_capacity_vector.h"
#include "Move_generator.h"
#include "Pieces.h"
#include "Transposition_table.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <limits>

namespace
{
	using namespace engine;

	using weightmap_type = Piece_map<std::array<int, board_size*board_size>>;
	constexpr Side_map<weightmap_type> weightmaps = []() constexpr
	{
		Side_map<weightmap_type> weightmaps{};
		weightmaps[Side::white][Piece::pawn] =
		{
			 0,  0,  0,  0,  0,  0,  0,  0,
			 5, 10, 10,-20,-20, 10, 10,  5,
			 5, -5,-10,  0,  0,-10, -5,  5,
			 0,  0,  0, 20, 20,  0,  0,  0,
			 5,  5, 10, 25, 25, 10,  5,  5,
			 10, 10, 20, 30, 30, 20, 10, 10,
			 50, 50, 50, 50, 50, 50, 50, 50,
			 0,  0,  0,  0,  0,  0,  0,  0
		};
		weightmaps[Side::white][Piece::knight] =
		{
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-50,-40,-30,-30,-30,-30,-40,-50
		};
		weightmaps[Side::white][Piece::bishop] =
		{
			-20,-10,-10,-10,-10,-10,-10,-20,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-20,-10,-10,-10,-10,-10,-10,-20
		};
		weightmaps[Side::white][Piece::rook] =
		{
			 0,  0,  5,  10, 10, 5,  0,  0,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			 5,  10, 10, 10, 10, 10, 10, 5,
			 0,  0,  0,  0,  0,  0,  0,  0,
		};
		weightmaps[Side::white][Piece::queen] =
		{
			-20,-10,-10, -5, -5,-10,-10,-20,
			-10,  0,  5,  0,  0,  0,  0,-10,
			-10,  5,  5,  5,  5,  5,  0,-10,
			 0,  0,  5,  5,  5,  5,  0, -5,
			-5,  0,  5,  5,  5,  5,  0, -5,
			-10,  0,  5,  5,  5,  5,  0,-10,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-20,-10,-10, -5, -5,-10,-10,-20,
		};
		weightmaps[Side::white][Piece::king] =
		{
			 20,  30,  10,  0,   0,   10,  30,  20,
			 20,  20,  0,   0,   0,   0,   20,  20,
			-10, -20, -20, -20, -20, -20, -20, -10,
			-20, -30, -30, -40, -40, -30, -30, -20,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
		};

		weightmaps[Side::black] = weightmaps[Side::white];
		for(auto& weightmap : weightmaps[Side::black])
		{
			for(std::uint8_t rank{0}; rank<board_size/2; ++rank)
			{
				for(std::uint8_t file{0}; file<board_size; ++file)
				{
					Position above_midpoint = Position{rank, file};
					Position below_midpoint = Position{static_cast<std::uint8_t>(board_size-1-rank), file};
					std::swap(weightmap[to_index(above_midpoint)], weightmap[to_index(below_midpoint)]);
				}
			}
		}
		return weightmaps;
	}();

	Piece_map<int> piece_values = []()
	{
		Piece_map<int> piece_values{};
		piece_values[Piece::pawn]   = 100;
		piece_values[Piece::knight] = 288;
		piece_values[Piece::bishop] = 345;
		piece_values[Piece::rook]   = 480;
		piece_values[Piece::queen]  = 1077;
		return piece_values;
	}();

	[[nodiscard]] double evaluate(const State& state) noexcept
	{
		const auto side_evaluation = [&state](const Side& side)
		{
			double total{0};
			for(const auto& piece : all_pieces)
			{
				for(std::uint8_t index{0}; index<board_size*board_size; ++index)
				{
					const Position current_square{index};
					if(!is_free(current_square, state.sides[side].pieces[piece])) [[unlikely]]
						total += weightmaps[side][piece][index] + piece_values[piece];
				}
			}
			return total;
		};
		return side_evaluation(state.side_to_move) - side_evaluation(other_side(state.side_to_move));
	}
}

namespace engine
{
	Move generate_best_move(State state, const uci::Search_options& search_options)
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

			const double stand_pat = evaluate(state);
			double best_score = stand_pat;
			if(stand_pat >= beta || additional_depth >= 3)
				return stand_pat;
			if(alpha < stand_pat)
				alpha = stand_pat;

			for(const auto& move : generate_moves<Moves_type::noisy>(state))
			{
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
					return cache_result->eval;
				if(cache_result->search_result_type == Search_result_type::lower_bound)
					alpha = std::max(alpha, cache_result->eval);
				if(cache_result->search_result_type == Search_result_type::upper_bound)
					beta = std::min(beta, cache_result->eval);
				if(alpha >= beta)
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
			std::array<Fixed_capacity_vector<Move, 256>, 2> child_pvs;
			int best_child_pv_index{0};
			for(const auto& move : all_legal_moves)
			{
				if(stop_searching())
					throw timeout{};

				const auto inferior_child_pv_index = (best_child_pv_index+1)%2;
				auto& inferior_child_pv = child_pvs[inferior_child_pv_index];
				inferior_child_pv.clear();
				state.make(move);
				double score = -rec(remaining_depth-1, extended_depth, nodes, depth, inferior_child_pv, -beta, -alpha);
				state.unmove();
				if(score > best_score)
				{
					best_score = score;
					best_move = move;
					best_child_pv_index = inferior_child_pv_index;
				}
				if(score > alpha)
				{
					alpha = score;
					if(alpha >= beta)
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

			if(const auto& best_child_pv = child_pvs[best_child_pv_index]; !best_child_pv.empty())
			{
				pv.clear();
				pv.push_back(best_move);
				pv.insert(pv.end(), best_child_pv.begin(), best_child_pv.end());
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
