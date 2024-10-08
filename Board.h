#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Constants.h"
#include "FEN.h"
#include "Pieces.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>

namespace engine
{
	// TODO - fix broken lambda add_piece
	struct Board
	{
		explicit inline Board(engine::FEN fen)
		{
			const auto FEN_string = fen.state_string();
			const auto to_absolute_index = [](std::size_t board_index)
			{
				const auto rank = board_index/engine::board_size;
				const auto file = board_index%engine::board_size;
				return (engine::board_size-1-rank)*engine::board_size+file;
			};
			
			std::size_t board_index{};
			for(std::size_t i{0}; i<FEN_string.size(); ++i)
			{
				const auto add_piece = [&](std::array<uint64_t, 6>& pieces)
				{
					std::size_t piece_type_index = static_cast<std::size_t> (fen.to_piece(std::tolower(FEN_string[i])));
					pieces[piece_type_index] |= 1ULL << to_absolute_index(board_index);
				};

				if(FEN_string[i] == '/')
					continue;
				else if(std::isdigit(FEN_string[i]))
				{
					board_index+=FEN_string[i]-'0';
				}
				else if(std::isupper(FEN_string[i]))
				{
					add_piece(white_pieces);
					++board_index;
				}
				else
				{
					add_piece(black_pieces);
					++board_index;
				}
			}
		}

		inline void output() const
		{
			//TODO - currently uses old piece type, update to use bitboard.
			/*
			const auto enumerate_pieces = [&]() -> std::array<char, board_size*board_size>
			{
				const auto to_character = [](Piece piece) -> char
				{
					constexpr std::array<char, 8> map = {'p', 'n', 'b', 'r', 'q', 'k'};
					return map[static_cast<int>(piece)];
				};
				std::array<char, board_size*board_size> board;
				std::ranges::fill(board, ' ');
				for(std::size_t i{0}; i<white_pieces.size(); ++i)
				{
					if(white_pieces[i])
						board[i] = std::toupper(to_character(white_pieces[i].value()));
				}
				for(std::size_t i{0}; i<black_pieces.size(); ++i)
				{
					if(black_pieces[i])
						board[i] = std::tolower(to_character(black_pieces[i].value()));
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
			*/
		}

		std::array<uint64_t, 6> white_pieces{};
		std::array<uint64_t, 6> black_pieces{};

		bool black_can_castle_left, black_can_castle_right;
		bool white_can_castle_left, white_can_castle_right;

		std::array<uint64_t, 6> squares_occupied_by_black;
		std::array<uint64_t, 6> squares_occupied_by_white;
		std::array<uint64_t, 6> occupied_squares;
	};
}

#endif // Board_h_INCLUDED
