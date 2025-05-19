#include "Bitboard.h"
#include "Constants.h"
#include "Engine.h"
#include "Move_generator.h"
#include "Pieces.h"
#include "Transposition_table.h"

#include <array>
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
	std::optional<Move> generate_move_at_depth(State state, const unsigned depth) noexcept
	{
		Transposition_table transposition_table(27);
		const auto quiescence_search = [&state](this auto&& rec, double alpha, double beta) -> double
		{
			const double stand_pat = evaluate(state);
			double best_score = stand_pat;
			if(stand_pat >= beta)
				return stand_pat;
			if(alpha < stand_pat)
				alpha = stand_pat;

			for(const engine::Move& move : noisy_moves(state))
			{
				state.make(move);
				const double score = -rec(-beta, -alpha);
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

		struct Repetition_data
		{
			engine::Move our_move, their_move;
			unsigned our_count, their_count;
		};

		const auto inverted_repetition_data = [](const Repetition_data& repetition_data)
		{
			return Repetition_data{repetition_data.their_move, repetition_data.our_move, repetition_data.their_count, repetition_data.our_count};
		};

		const auto nega_max = [&state, &quiescence_search, &transposition_table, &inverted_repetition_data](this auto&& rec, const unsigned current_depth, std::optional<Move>& best_move, Repetition_data repetition_data, double alpha = -std::numeric_limits<double>::infinity(), double beta = std::numeric_limits<double>::infinity()) -> double
		{
			if(current_depth == 0)
			{
				const auto current_hash = zobrist::hash(state);
				if(auto& cache_data = transposition_table[state]; cache_data && cache_data->depth >= 0 && cache_data->zobrist_hash == current_hash)
					return cache_data->eval;
				else
				{
					const double evaluation = quiescence_search(alpha, beta);
					cache_data = Transposition_data{current_depth, evaluation, state.side_to_move, current_hash};
					return evaluation;
				}
			}

			std::optional<double> best_seen_score = std::nullopt;
			for(const auto& move : legal_moves(state))
			{
				state.make(move);
				std::optional<Move> opponent_move;
				double score;
				const auto current_hash = zobrist::hash(state);
				if(auto& cache_result = transposition_table[state]; cache_result && cache_result->depth >= current_depth && cache_result->zobrist_hash == current_hash)
					score = state.side_to_move==cache_result->to_move? cache_result->eval : -cache_result->eval;
				else
				{
					if(move == repetition_data.our_move && ++repetition_data.our_count >= 3)
						score = 0.0;
					else
					{
						repetition_data.our_move = move;
						repetition_data.our_count = 1;
						score = -rec(current_depth-1, opponent_move, inverted_repetition_data(repetition_data) , -beta, -alpha);
					}
					cache_result = Transposition_data{current_depth, score, state.side_to_move, current_hash};
				}
				state.unmove();
				if(!best_seen_score || score > best_seen_score.value())
				{
					best_seen_score = score;
					best_move = move;
				}
				if(score >= beta)
					break;
				if(score >= alpha)
					alpha = score;
			}
			if(best_seen_score)
				return best_seen_score.value();
			else
			{
				if(state.is_square_attacked(state.sides[state.side_to_move].pieces[Piece::king].lsb_square()))
					return 0.0;
				else
					return -std::numeric_limits<double>::infinity();
			}
		};
		std::optional<Move> best_move;
		for(unsigned current_depth{0}; current_depth < depth; ++current_depth)
		{
			nega_max(current_depth, best_move, Repetition_data{Move{}, Move{}, 0, 0});
		}
		return best_move;
	}
}
