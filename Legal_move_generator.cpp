#include "Legal_move_generator.h"

#include "Board_state.h"
#include "Constants.h"
#include "Position.h"
#include "Square.h"

using namespace engine;

//will fix constexpr
uint64_t Legal_move_generator::reachable_squares(const Square& square, const Piece& piece, const uint64_t& occupied_squares)
{
	//TODO
	switch(piece)
	{
		case Piece::pawn:
			break;
		case Piece::knight:
			break;
		case Piece::bishop:
			break;
		case Piece::rook:
			break;
		case Piece::queen:
			break;
		case Piece::king:
			break;
	}
}

std::optional<std::vector<Move>> Legal_move_generator::knight_reachable_squares(const Square& knight_square, const uint64_t& occupied_squares)
{
	std::optional<std::vector<Move>> valid_moves = std::nullopt;
	uint64_t reachable_squares{0};
	for(const auto& move : knight_moves)
	{
		const auto [del_rank, del_file] = move;
		const Square destination_square = static_cast<Square>(static_cast<std::size_t>(knight_square)+del_rank*board_size+del_file);
		if(is_valid_destination(destination_square, occupied_squares))
		{
			if(!valid_moves)
			{
				valid_moves.emplace();
			}
			valid_moves->push_back(Move{knight_square, destination_square});
		}
	}
	return valid_moves;
}

std::optional<std::vector<Move>> Legal_move_generator::rook_reachable_squares(const Square& square, const uint64_t& occupied_squares)
{

}

std::optional<std::vector<Move>> Legal_move_generator::bishop_reachable_squares(const Square& square, const uint64_t& occupied_squares)
{

}

void Legal_move_generator::initialise_attack_table()
{
	
}

void Legal_move_generator::cast_magic()
{

}
