#include "Move_generator.h"
#include "Position.h"

using namespace engine;

constexpr Move_generator::Move_generator()
{
	initialise_attack_table();
	cast_magic();
}
//fix to return MOVES.
const std::unordered_map<Piece, Bitboard> Move_generator::get(const Side_position& side, const Side& active_player) const
{
	std::unordered_map<Piece, Bitboard> legal_moves;
	pawn_legal_moves(side.pieces[static_cast<std::size_t>(Piece::knight)], side.occupied_squares, active_player);
	knight_legal_moves(side.pieces[static_cast<std::size_t>(Piece::knight)], side.occupied_squares);
}

const Bitboard Move_generator::pawn_legal_moves(const Bitboard& white_pawns, const Bitboard& occupied_squares, const Side& active_player) const
{
	Bitboard valid_moves{0};
	const auto rank = 0xFF;
	const auto pawn_direction = active_player == Side::white ? 1 : -1;
	const auto starting_rank = active_player == Side::white ? 1 : 6;
	const auto promotion_rank = active_player == Side::white ? 7 : 0;
	const auto promotion_mask = rank << (promotion_rank*board_size);

	Bitboard single_moves = (white_pawns << board_size*pawn_direction) & ~occupied_squares;
	valid_moves |= single_moves;

	const auto rank_mask = rank << ((starting_rank+pawn_direction)*board_size);
	Bitboard double_moves = (single_moves & rank_mask) << board_size*pawn_direction;
	valid_moves |= double_moves;

	Bitboard left_captures = (single_moves << -1) & occupied_squares & ~file_h;
	Bitboard right_captures = (single_moves << 1) & occupied_squares & ~file_a;
	valid_moves |= left_captures | right_captures;

	Bitboard promotion_moves = single_moves & promotion_mask;
	valid_moves |= promotion_moves;

	return valid_moves;
}

const Bitboard Move_generator::knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares) const
{
	Bitboard valid_moves{0};
	knight_bb.for_each_piece([&](const Position& knight_square)
	{
		for(const auto& move : knight_moves)
		{
			const auto [del_rank, del_file] = move;
			const Position destination_square(knight_square.rank_+del_rank, knight_square.file_+del_file);
			if(is_valid_destination(destination_square, occupied_squares))
				valid_moves |= 1 << to_index(destination_square);
		}
	});
	return valid_moves;
}

const Bitboard Move_generator::king_legal_moves(const Position& king_square, const Bitboard& occupied_squares, Side& active_player) const
{
	Bitboard valid_moves{0};
	for(const auto& move : king_moves)
	{
		const auto [del_rank, del_file] = move;
		const Position destination_square(king_square.rank_+del_rank, king_square.file_+del_file);
		if(is_valid_destination(destination_square, occupied_squares))
			valid_moves |= 1 << to_index(destination_square);
	}
	return valid_moves;
}

constexpr const Bitboard Move_generator::rook_reachable_squares(const Position& rook_square, const Bitboard& occupied_squares) const
{
	Bitboard valid_moves{};
	for(int del_rank{0}; del_rank < board_size; ++del_rank)
	{
		for(int del_file{0}; del_file < board_size; ++del_file)
		{
			const Position destination_square = rook_square + Position{del_rank, del_file};
			if(is_valid_destination(destination_square, occupied_squares))
				valid_moves |= (1 << to_index(destination_square));
			else 
				break;
		}
	}
	return valid_moves;
}

constexpr const Bitboard Move_generator::bishop_reachable_squares(const Position& bishop_square, const Bitboard& occupied_squares) const
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
	for(const auto& direction : bishop_moves)
	{
		explore_diagonal(bishop_square, direction, occupied_squares, valid_moves, bishop_square);
	}
	return valid_moves;
}

constexpr void Move_generator::initialise_attack_table()
{
	for(const auto& moves : bishop_moves())
	{

	}
}

constexpr void Move_generator::cast_magic()
{

}
