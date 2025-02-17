#include "Move_generator.h"
#include "Position.h"
#include "Utility.h"

using namespace engine;

namespace std 
{
	template <>
	struct hash<Position>
	{
		size_t operator()(const Position& p) const
		{
			size_t h1 = hash<int>{}(p.rank_);
			size_t h2 = hash<int>{}(p.file_);
			return h1 ^ (h2 << 1);
		}
	};
}

/*
const std::array<Move_generator::moves_type, number_of_piece_types> Move_generator::get(const Side_position& side, const Side& active_player) const
{
	std::array<moves_type, number_of_piece_types> legal_moves{};
	pawn_legal_moves(side.pieces[static_cast<std::size_t>(Piece::pawn)], side.occupied_squares, active_player);
	knight_legal_moves(side.pieces[static_cast<std::size_t>(Piece::knight)], side.occupied_squares);
}
*/
const Move_generator::moves_type Move_generator::pawn_legal_moves(const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player)
{
	moves_type legal_moves{};
	const auto rank = 0xFF;
	const auto pawn_direction = active_player == Side::white ? 1 : -1;
	const auto starting_rank = active_player == Side::white ? 1 : 6;
	const auto promotion_rank = active_player == Side::white ? 7 : 0;
	const auto promotion_mask = rank << (promotion_rank*board_size);

	pawns_bb.for_each_piece([&](const Position& original_square)
	{
		Bitboard pawn_bb{1ULL << to_index(original_square)};
		Bitboard valid_moves{0};
		Bitboard single_moves = (pawn_bb << board_size*pawn_direction) & ~occupied_squares;
		valid_moves |= single_moves;

		const auto rank_mask = rank << ((starting_rank+pawn_direction)*board_size);
		Bitboard double_moves = (single_moves & rank_mask) << board_size*pawn_direction;
		valid_moves |= double_moves;

		Bitboard left_captures = (single_moves << -1) & occupied_squares & ~file_h;
		Bitboard right_captures = (single_moves << 1) & occupied_squares & ~file_a;
		valid_moves |= left_captures | right_captures;

		Bitboard promotion_moves = single_moves & promotion_mask;
		valid_moves |= promotion_moves;
		valid_moves.for_each_piece([&legal_moves, &original_square](const Position& destination_square)
		{
			legal_moves[original_square].push_back(destination_square);
		});
	});
	return legal_moves;
}

const Move_generator::moves_type Move_generator::knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares)
{
	moves_type legal_moves;
	knight_bb.for_each_piece([occupied_squares, &legal_moves](const Position& original_square)
	{
		for(const auto& move : knight_moves_)
		{
			const auto [del_rank, del_file] = move;
			const Position destination_square(original_square.rank_+del_rank, original_square.file_+del_file);
			if(is_valid_destination(destination_square, occupied_squares))
				legal_moves[original_square].push_back(destination_square);
		}
	});
	return legal_moves;
}

const Move_generator::moves_type Move_generator::king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares)
{
	moves_type legal_moves;
	const Position original_square = king_bb.lsb_index();
	for(const auto& move : king_moves_)
	{
		const auto [del_rank, del_file] = move;
		const Position destination_square(original_square.rank_+del_rank, original_square.file_+del_file);
		if(is_valid_destination(destination_square, occupied_squares))
			legal_moves[original_square].push_back(destination_square);
	}
	return legal_moves;
}

const Bitboard Move_generator::rook_reachable_squares(const Position& original_square, const Bitboard& occupied_squares)
{
	Bitboard valid_moves{};
	for(int del_rank{0}; del_rank < board_size; ++del_rank)
	{
		for(int del_file{0}; del_file < board_size; ++del_file)
		{
			const Position destination_square = original_square + Position{del_rank, del_file};
			if(is_valid_destination(destination_square, occupied_squares))
				valid_moves |= (1 << to_index(destination_square));
			else 
				return valid_moves;
		}
	}
	return valid_moves;
}

const Bitboard Move_generator::bishop_reachable_squares(const Position& bishop_square, const Bitboard& occupied_squares)
{
	const auto explore_diagonal = [](this auto&& rec, const Position& bishop_square, const Position& diagonal_offset,
		const Bitboard occupied_squares, Bitboard& valid_moves,
		const Position& original_bishop_square)
	{
		const Position destination_square = bishop_square + diagonal_offset;
		if(!is_valid_destination(destination_square, occupied_squares))
			return;
		valid_moves |= (1 << to_index(destination_square));

		rec(destination_square, diagonal_offset, occupied_squares, valid_moves, original_bishop_square);
	};

	Bitboard valid_moves{};
	for(const auto& direction : bishop_moves_)
	{
		explore_diagonal(bishop_square, direction, occupied_squares, valid_moves, bishop_square);
	}
	return valid_moves;
}

void Move_generator::cast_magic()
{

}
