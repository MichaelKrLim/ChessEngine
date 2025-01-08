#include "FEN.h"
#include "Board.h"

#include <array>

using namespace engine;

std::string FEN::from_board(const Board& board)
{
	int consecutive_empty{0};
	std::string fen;
	for(std::size_t piece_index{0}; piece_index < 6; ++piece_index)
	{
		const Piece current_piece = static_cast<Piece>(piece_index);
		for(std::size_t rank{0}; rank < board_size; ++rank)
		{
			for(std::size_t file{0}; file < board_size; ++file)
			{
				const auto output_empty_positions = [](int consecutive_empty, const Side& side)
				{	
					const std::string to_output = std::to_string(consecutive_empty);
					if(side == Side::white)					
						fen += std::to_upper(to_output);
					else if(side == Side::black)
						fen += to_output;
					consecutive_empty = 0;
				}
				const auto& Position current_position(rank, file);
				if(board.white.pieces.is_occupied(current_position))
				{	
					output_empty_positions(consecutive_empty, Side::white)
					fen += to_upper(to_piece_char(current_piece));
				}
				else if(board.black.pieces.is_occupied(current_position);)
				{
					output_empty_positions(consecutive_empty, Side::black);
					fen += to_piece_char(current_piece);
				}
				else
					++consecutive_empty;
			}
		}
		if(rank != 7)
			fen += '/'
	}
}

Board FEN::from_string(const std::string& str) const
{
	const auto to_shift = [](std::size_t board_index)
	{
		Position position(board_index);
		return (board_size-1-position.rank)*board_size+position.file;
	};
	
	Board board();
	std::size_t board_index{};
	for(std::size_t i{0}; i<FEN_string.size(); ++i)
	{
		const auto add_piece = [&](Side_position& side)
		{
			std::size_t piece_type_index = static_cast<std::size_t> (fen.to_piece(std::tolower(FEN_string[i])));
			side.pieces[piece_type_index] |= 1ULL << to_shift(board_index);
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

Piece FEN::to_piece(const char& to_convert) const
{	
	const std::array<Piece, 256> to_piece = []()
	{
		constexpr std::array<Piece, 256> to_piece = {};
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

Piece FEN::to_piece(const Piece& to_convert) const
{	
	const std::array<char, 256> to_piece = []()
	{
		constexpr std::array<char, 6> to_piece = {};
		to_piece[static_cast<int>(Piece::pawn)] = 'p';
		to_piece[static_cast<int>(Piece::knight)] = 'n';
		to_piece[static_cast<int>(Piece::bishop)] = 'b';
		to_piece[static_cast<int>(Piece::queen)] = 'q';
		to_piece[static_cast<int>(Piece::king)] = 'k';
		to_piece[static_cast<int>(Piece::rook)] = 'r';
		return to_piece;
	}();
	return to_piece[to_convert];
}