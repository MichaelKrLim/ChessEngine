#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Bitboard.h"
#include "Constants.h"
#include "Pieces.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>

namespace engine
{
	struct Side_position
	{
		std::array<Bitboard, 6> pieces{};
		bool can_castle_left{true}, can_castle_right{true};
		Bitboard occupied_squares{};
	};

	struct Board
	{
		public:

		explicit inline Board(const std::string& FEN_string)
		{
			const auto to_shift = [](std::size_t board_index) -> std::size_t
			{
				Position position(board_index);
				const std::size_t flipped = (board_size-1-position.rank_)*board_size+position.file_;
				return flipped;
			};
			
			std::size_t board_index{0};
			for(std::size_t i{0}; i<FEN_string.size(); ++i)
			{
				const auto add_piece = [&](Side_position& side)
				{
					std::size_t piece_type_index = to_piece_index(std::tolower(FEN_string[i]));
					const std::uint64_t mask = (1ULL << to_shift(board_index));
					side.pieces[piece_type_index] |= mask;
					side.occupied_squares |= mask;
					occupied_squares |= mask;
				};
				if(FEN_string[i] == '/')
					continue;
				else if(std::isdigit(FEN_string[i]))
					board_index+=FEN_string[i]-'0';
				else if(std::isupper(FEN_string[i]))
				{
					add_piece(white);
					++board_index;
				}
				else
				{
					add_piece(black);
					++board_index;
				}
			}
		}
		inline Board() = default;

		Side_position black{};
		Side_position white{};
		Bitboard occupied_squares{};

		[[nodiscard]] std::string FEN(const Board& board)
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

		private:

		[[nodiscard]] constexpr std::size_t to_piece_index(const char& to_convert)
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
			return static_cast<std::size_t>(to_piece[to_convert]);
		}

		[[nodiscard]] constexpr char to_piece(const int& to_convert)
		{	
			constexpr std::array<char, 6> to_piece{'p', 'n', 'b', 'q', 'k', 'r'};
			return to_piece[to_convert];
		}
	};
}

#endif // Board_h_INCLUDED