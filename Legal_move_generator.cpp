#include "Legal_move_generator.h"

#include "Constants.h"
#include "Position.h"

using namespace engine;

constexpr Legal_move_generator::Legal_move_generator()
{
	cast_magic();	
}

std::uint64_t Legal_move_generator::kings_reachable_squares(const Position& knight_square, const std::uint64_t& occupied_squares)
{
	std::uint64_t valid_moves{0};
	for(const auto& move : king_moves)
	{
		const auto [del_rank, del_file] = move;
		const Position destination_square{king_square.rank_+del_rank, king_square.file_+del_file};
		valid_moves |= (1 << to_index(destination_square))
	}
}

std::uint64_t Legal_move_generator::knights_reachable_squares(const Position& knight_square, const std::uint64_t& occupied_squares)
{
	uint64_t valid_moves{0};
	for(const auto& move : knight_moves)
	{
		const auto [del_rank, del_file] = move;
		const Position destination_square{knight_square.rank_+del_rank, knight_square.file_+del_file};
		valid_moves |= (1 << to_index(destination_square))
	}
	return valid_moves;
}

constexpr std::uint64_t Legal_move_generator::rooks_reachable_squares(const Position& rook_square, const std::uint64_t& occupied_squares)
{
	std::uint64_t valid_moves{0};
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

constexpr std::vector<Move> Legal_move_generator::bishops_reachable_squares(const Position& bishop_square, const uint64_t& occupied_squares)
{
	std::uint64_t valid_moves{};
	for(const auto& direction : bishop_moves)
	{
		explore_diagonal(bishop_square, direction, occupied_squares, valid_moves, bishop_square);
	}
	return valid_moves;
}



constexpr void Legal_move_generator::explore_diagonal(const Position& bishop_square, const Position& diagonal_offset, const uint64_t occupied_squares, std::uint64_t& valid_moves, const Position& origional_bishop_square)
{
	const Position destination_square = bishop_square + diagonal_offset;
	if(!is_valid_destination(destination_square, occupied_squares))
		return;
	valid_moves |= (1 << destination_square);

	explore_diagonal(destination_square, diagonal_offset, occupied_squares, valid_moves, origional_bishop_square);
}

std::vector<Move> legal_pawn_moves()
{
	
}

std::vector<Move> get()
{

}

constexpr void Legal_move_generator::initialise_attack_table()
{
	for(const auto& moves : { {bishop_moves()}

	})
	{

	}
}

constexpr void Legal_move_generator::cast_magic()
{

}
