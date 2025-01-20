#include "Bitboard.h"
#include "Board.h"
#include "FEN.h"

#include <array>

using namespace engine;

std::string FEN::from_board(const Board& board) const
{
	int consecutive_empty{0};
	std::string fen{};
	for(int piece_index{0}; piece_index < 6; ++piece_index)
	{
		for(std::size_t rank{0}; rank < board_size; ++rank)
		{
			for(std::size_t file{0}; file < board_size; ++file)
			{
				const auto output_empty_positions = [&fen](int& consecutive_empty, const Side& side)
				{	
					fen += std::to_string(consecutive_empty);
					consecutive_empty = 0;
				};
				const Position current_position(rank, file);
				if(board.white.pieces[piece_index].is_occupied(current_position))
				{	
					output_empty_positions(consecutive_empty, Side::white);
					fen += std::toupper(to_piece(piece_index));
				}
				else if(board.black.pieces[piece_index].is_occupied(current_position))
				{
					output_empty_positions(consecutive_empty, Side::black);
					fen += to_piece(piece_index);
				}
				else
					++consecutive_empty;
			}
			if(rank != 7)
				fen += '/';
		}
	}
	return fen;
}

Board FEN::from_string(const std::string& FEN_string) const
{
	const auto to_shift = [](std::size_t board_index)
	{
		Position position(board_index);
		return (board_size-1-position.rank_)*board_size+position.file_;
	};
	
	Board board{};
	std::size_t board_index{};
	for(std::size_t i{0}; i<FEN_string.size(); ++i)
	{
		const auto add_piece = [&](Side_position& side)
		{
			std::size_t piece_type_index = static_cast<std::size_t>(to_piece(std::tolower(FEN_string[i])));
			side.pieces[piece_type_index] |= (1ULL << to_shift(board_index));
			side.occupied_squares |= side.pieces[piece_type_index];
		};

		if(FEN_string[i] == '/')
			continue;
		else if(std::isdigit(FEN_string[i]))
			board_index+=FEN_string[i]-'0';
		else if(std::isupper(FEN_string[i]))
		{
			add_piece(board.white);
			++board_index;
		}
		else
		{
			add_piece(board.black);
			++board_index;
		}
	}
	board.occupied_squares = board.black.occupied_squares | board.white.occupied_squares;
	return board;
}

constexpr Piece FEN::to_piece(const char& to_convert) const
{
	constexpr auto to_piece = []()
	{
		std::array<Piece, 256> to_piece = {};
		to_piece['p'] = Piece::pawn;
		to_piece['n'] = Piece::knight;
		to_piece['b'] = Piece::bishop;
		to_piece['q'] = Piece::queen;
		to_piece['k'] = Piece::king;
		to_piece['r'] = Piece::rook;
		return to_piece;
	}();
	return to_piece[to_convert];
}

constexpr char FEN::to_piece(const int& to_convert) const
{	
	constexpr std::array<char, 6> to_piece{'p', 'n', 'b', 'q', 'k', 'r'};
	return to_piece[to_convert];
}
