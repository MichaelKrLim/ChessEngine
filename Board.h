#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Constants.h"
#include "FEN.h"
#include "Pieces.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <ranges>

namespace engine
{
	struct Side_position
	{
		std::array<uint64_t, 6> pieces{};
		bool can_castle_left, can_castle_right;
		//uint64_t occupied_squares{};
	};
	struct Board
	{
		//TODO - add castling rights and pawns for en passant
		explicit inline Board(FEN fen)
		{
			const auto FEN_string = fen.state_string();
			const auto to_absolute_index = [](std::size_t board_index)
			{
				const auto rank = board_index/engine::board_size;
				const auto file = board_index%engine::board_size;
				return (board_size-1-rank)*board_size+file;
			};
			
			std::size_t board_index{};
			for(std::size_t i{0}; i<FEN_string.size(); ++i)
			{
				const auto add_piece = [&](Side_position& side)
				{
					std::size_t piece_type_index = static_cast<std::size_t> (fen.to_piece(std::tolower(FEN_string[i])));
					side.pieces[piece_type_index] |= 1ULL << to_absolute_index(board_index);
					side.occupied_squares |= side.pieces[piece_type_index];
				};

				if(FEN_string[i] == '/')
					continue;
				else if(std::isdigit(FEN_string[i]))
				{
					board_index+=FEN_string[i]-'0';
				}
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
			occupied_squares = black.occupied_squares | white.occupied_squares;
		}

		inline void output() const
		{
			const auto enumerate_pieces = [&]() -> std::array<char, board_size*board_size>
			{
				const auto to_character = [](Piece piece) -> char
				{
					constexpr std::array<char, 8> map = {'k', 'p', 'n', 'b', 'r', 'q'};
					return map[static_cast<int>(piece)];
				};
				std::array<char, board_size*board_size> board;
				std::ranges::fill(board, ' ');
				for(std::size_t piece{0}; piece<number_of_piece_types; ++piece)
				{
					for(std::size_t board_index{0}; board_index<board_size*board_size; ++board_index)
					{
						if(((white.pieces[piece] >> board_index) & 1) == 1)
							board[board_index] = std::toupper(to_character(static_cast<Piece>(piece)));
					}
				}
				for(std::size_t piece{0}; piece<number_of_piece_types; ++piece)
				{
					for(std::size_t board_index{0}; board_index<board_size*board_size; ++board_index)
					{
						if(((black.pieces[piece] >> board_index) & 1) == 1)
							board[board_index] = std::tolower(to_character(static_cast<Piece>(piece)));
					}
				}
				return board;
			};
			const auto board = enumerate_pieces();
			for(int rank{board_size-1}; rank>=0; --rank)
			{
				for(int file{0}; file<board_size; ++file)
				{
					std::cout << board[rank*board_size+file];
				}
				std::cout << "\n";
			}
		}

		Side_position black;
		Side_position white;

		uint64_t occupied_squares{};
	};
}

#endif // Board_h_INCLUDED
