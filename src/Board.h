#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Bitboard.h"
#include "Constants.h"
#include "Move.h"
#include "Pieces.h"
#include "Position.h"

#include <array>
#include <cctype>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>

namespace engine
{
	struct Side_position
	{
		std::array<Bitboard, 6> pieces{};
		std::optional<Position> en_passent_target_square{std::nullopt};
		Bitboard occupied_squares{};
		std::array<bool, 2> castling_rights{false, false};
	};

	enum class Castling_rights
	{
		kingside, queenside
	};

	struct Board
	{
		public:

		explicit inline Board(const std::string& fen_string)
		{
			const auto to_shift = [](std::size_t board_index) -> std::size_t
			{
				Position position(board_index);
				const std::size_t flipped = (board_size-1-position.rank_)*board_size+position.file_;
				return flipped;
			};
			const auto read_placement_data = [&](const std::string& fen_section)
			{
				std::size_t board_index{0};
				for(std::size_t i{0}; i<fen_string.size(); ++i)
				{
					const auto add_piece = [&](const Side& side)
					{
						const auto side_index = static_cast<std::uint8_t>(side);
						std::size_t piece_type_index = to_piece_index(std::tolower(fen_string[i]));
						const std::uint64_t mask = (1ULL << to_shift(board_index));
						sides[side_index].pieces[piece_type_index] |= mask;
						sides[side_index].occupied_squares |= mask;
						occupied_squares |= mask;
					};
					if(fen_string[i] == '/')
						continue;
					else if(std::isdigit(fen_string[i]))
						board_index+=fen_string[i]-'0';
					else if(std::isupper(fen_string[i]))
					{
						add_piece(Side::white);
						++board_index;
					}
					else
					{
						add_piece(Side::black);
						++board_index;
					}
				}
			};
			auto sections = std::ranges::views::split(fen_string, ' ');
			assert(std::ranges::distance(sections) == 6 && "Invalid FEN string");
			auto it = sections.begin();
			Position en_passent_target_square;
			for(std::size_t i{0}; i<6; ++i)
			{
				std::string fen_segment{(*it).begin(), (*it).end()};
				switch(i)
				{
					case 0:
						read_placement_data(fen_segment);
						break;
					case 1:
						is_white_to_move = fen_segment[0]=='w'? true : false;
						break;
					case 2:
						for(const char& castling_right : fen_segment)
						{
							std::uint8_t side_index, castling_right_index;
							switch(castling_right)
							{
								case 'K':
									side_index = static_cast<std::uint8_t>(Side::white);
									castling_right_index = static_cast<std::uint8_t>(Castling_rights::kingside);
									break;
								case 'Q':
									side_index = static_cast<std::uint8_t>(Side::white);
									castling_right_index = static_cast<std::uint8_t>(Castling_rights::queenside);
									break;
								case 'k':
									side_index = static_cast<std::uint8_t>(Side::black);
									castling_right_index = static_cast<std::uint8_t>(Castling_rights::kingside);
									break;
								case 'q':
									side_index = static_cast<std::uint8_t>(Side::black);
									castling_right_index = static_cast<std::uint8_t>(Castling_rights::queenside);
									break;
							}
							sides[side_index].castling_rights[castling_right_index] = true;
						}
						break;
					case 3:
						if(fen_segment.size() == 1 && fen_segment[0] == '-') break;
						assert(fen_segment.size()==2 && isalpha(fen_segment[0]) && isdigit(fen_segment[1]) && "Invalid FEN string");
						en_passent_target_square = Position{algebraic_to_index(fen_segment)};
						std::size_t side_index;
						if(en_passent_target_square.rank_ == white_en_passant_target_rank-1)
							side_index = static_cast<std::uint8_t>(Side::white);
						else
							side_index = static_cast<std::uint8_t>(Side::black);
						sides[side_index].en_passent_target_square = en_passent_target_square;
						break;
					case 4:
						assert(fen_segment.size() == 1 && isdigit(fen_segment[0]) && "Invalid FEN string");
						half_move_clock = fen_segment[0]-'0';
						break;
					case 5:
						assert(fen_segment.size() == 1 && isdigit(fen_segment[0]) && "Invalid FEN string");
						full_move_clock = fen_segment[0]-'0';
						break;
				}
				++it;
			}
		}
		inline Board() = default;

		std::array<Side_position, 2> sides{};
		int half_move_clock{}, full_move_clock{};
		bool is_white_to_move{true};
		Bitboard occupied_squares{};

		[[nodiscard]] inline std::string FEN()
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
						if(sides[static_cast<std::uint8_t>(Side::white)].pieces[piece_index].is_occupied(current_position))
						{	
							output_empty_positions(consecutive_empty, Side::white);
							fen += std::toupper(to_piece(piece_index));
						}
						else if(sides[static_cast<std::uint8_t>(Side::black)].pieces[piece_index].is_occupied(current_position))
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

		inline void make(Move&& move)
		{
			if(move.move_type() == Move_type::promotion)
			{
				for(Side_position& side : sides)
				{
					Bitboard& pawn_bb = side.pieces[static_cast<std::uint8_t>(Piece::pawn)];
					pawn_bb.for_each_piece([&](const Position& occupied_square)
					{
						if(occupied_square == move.from_square())
						{
							pawn_bb &= ~(1ULL << to_index(occupied_square));
							side.pieces[static_cast<std::uint8_t>(move.promotion_piece())] &= (1 << to_index(move.destination_square()));
						}
					});
				}
			}
			else
			{
				for(Side_position& side : sides)
				{
					for(Bitboard& piece_bb : side.pieces)
					{
						piece_bb.for_each_piece([&piece_bb, &move](const Position& occupied_square) mutable
						{
							if(occupied_square == move.from_square())
							{
								piece_bb &= ~(1ULL << to_index(occupied_square));
								piece_bb |=  (1ULL << to_index(move.destination_square()));
							}
						});
					}
				}
			}
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
