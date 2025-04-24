#include "Bitboard.h"
#include "Constants.h"
#include "Engine.h"
#include "Move_generator.h"
#include "Pieces.h"

#include <array>
#include <limits>

using namespace engine;

constexpr Side_map<Engine::weightmap_type> Engine::weightmaps = []() constexpr
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

constexpr Piece_map<int> Engine::piece_values = []() constexpr
{
	Piece_map<int> piece_values{};
	piece_values[Piece::pawn]   = 100;
	piece_values[Piece::knight] = 288;
	piece_values[Piece::bishop] = 345;
	piece_values[Piece::rook]   = 480;
	piece_values[Piece::queen]  = 1077;
	return piece_values;
}();

double Engine::evaluate(const Board& board) noexcept
{
	const auto side_evaluation = [&board](const Side& side)
	{
		double total{0};
		for(const auto& piece : all_pieces)
		{
			for(std::uint8_t index{0}; index<board_size*board_size; ++index)
			{
				const Position current_square{index};
				if(!is_free(current_square, board.sides[side].pieces[piece])) [[unlikely]]
					total += weightmaps[side][piece][index] + piece_values[piece];
			}
		}
		return total;
	};
	return side_evaluation(board.side_to_move) - side_evaluation(other_side(board.side_to_move));
}

Move Engine::generate_move_at_depth(Board board, const int depth) noexcept
{
	const auto nega_max = [&board, this](this auto&& rec, int depth, Move& best_move, double alpha = -std::numeric_limits<double>::infinity(), double beta = std::numeric_limits<double>::infinity())
	{
		if(depth == 0) 
			return evaluate(board);

		std::optional<double> best_seen_score = std::nullopt;
		for(const auto& move : legal_moves(board))
		{
			board.make(move);
			Move opponent_move;
			double score = -rec(depth-1, opponent_move, -beta, -alpha);
			board.unmove();
			if(!best_seen_score || score > best_seen_score.value())
			{
				best_seen_score = score;
				best_move = move;
			}
			alpha = std::max(alpha, best_seen_score.value());
			if(alpha >= beta)
				break;
		}
		if(best_seen_score)
			return best_seen_score.value();
		else
			return -std::numeric_limits<double>::infinity();
	};
	Move best_move;
	std::cout << nega_max(depth, best_move) << ' ';
	return best_move;
}