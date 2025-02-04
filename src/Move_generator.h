#ifndef Bitboards_h_INCLUDED
#define Bitboards_h_INCLUDED

#include "Bitboard.h"
#include "Board.h"
#include "Constants.h"
#include "Move.h"
#include "Pieces.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace engine 
{
	class Move_generator
	{
		public:

		constexpr Move_generator();
		using moves_type = std::unordered_map<Position, std::vector<Position>>;
		[[nodiscard]] const std::array<moves_type, number_of_piece_types> get(const Side_position& side, const Side& active_player) const;
		
		private:

		consteval static std::array<Bitboard, 5000> create_attack_table();
		template <std::size_t size>
		consteval static std::array<Bitboard, size> Move_generator::blocker_configurations(const Position& square, const bool& bishop);
		constexpr void cast_magic();

		const moves_type pawn_legal_moves(const Bitboard& pawn_bb, const Bitboard& occupied_squares, const Side& active_player) const;
		const moves_type king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares) const;
		const moves_type knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares) const;

		constexpr static Bitboard rook_reachable_squares(const Position& square, const Bitboard& occupied_squares);
		constexpr static Bitboard bishop_reachable_squares(const Position& square, const Bitboard& occupied_squares);

		struct Magic_square
		{
			const std::vector<Bitboard>& attack_table;
			const Bitboard mask;
			const Bitboard magic;
			const int shift;
		};

		constexpr static std::array<Magic_square, 64> bishop_magic_squares_;
		constexpr static std::array<Magic_square, 64> rook_magic_squares_;
		constexpr static std::array<Bitboard, 5000> attack_table_ = create_attack_table();

		constexpr static std::array<Position, 8> knight_moves_ =
		{{
			Position{2, 1}, Position{2, -1}, Position{-2, 1}, Position{-2, -1},
			Position{1, 2}, Position{1, -2}, Position{-1, 2}, Position{-1, -2}
		}};
		constexpr static std::array<Position, 4> bishop_moves_ =
		{{
			Position{1, 1}, Position{1, -1},
			Position{-1, 1}, Position{-1, -1}
		}};
		constexpr static std::array<Position, 8> king_moves_ =
		{{
			Position{1, 0}, Position{-1, 0}, Position{0, 1}, Position{0, -1},
			Position{1, 1}, Position{-1, 1}, Position{1, -1},Position{-1, -1}
		}};
	};

	template <std::size_t size>
	consteval std::array<Bitboard, size> Move_generator::blocker_configurations(const Position& square, const bool& bishop)
	{
		Bitboard current_configuration{};
		std::array<Bitboard, size> blocker_configurations{};
		const auto add_blocker = [&](const Position& blocker_position) constexpr -> bool
		{
			if(is_on_board(blocker_position))
			{
				current_configuration |= to_index(blocker_position);
				return true;
			}
			else
				return false;
		};
		std::size_t index{0};
		for(std::size_t first_offset{}; add_blocker(square+(bishop? Position{1*first_offset, -1*first_offset} : Position{first_offset, 0})); ++first_offset)
		{
			for(std::size_t second_offset{}; add_blocker(square+(bishop? Position{-1*second_offset, -1*second_offset} : Position{second_offset, 0})); ++second_offset)
			{
				for(std::size_t third_offset{}; add_blocker(square+(bishop? Position{1*third_offset, 1*third_offset} : Position{0, third_offset})); ++third_offset)
				{
					for(std::size_t fourth_offset{}; add_blocker(square+(bishop? Position{-1*fourth_offset, 1*fourth_offset} : Position{0, fourth_offset})); ++fourth_offset)
					{
						blocker_configurations[index++] = current_configuration;
						current_configuration = 0;
					}
				}
			}
		}
		return blocker_configurations;
	}

	consteval std::array<Bitboard, 5000> Move_generator::create_attack_table()
	{
		std::array<Bitboard, 5000> attack_table{};
		std::size_t index{0};
		for(std::size_t rank{0}; rank<board_size; ++rank)
		{
			for(std::size_t file{0}; file<board_size; ++file)
			{
				const Position current_square = Position{rank, file};
				for(const auto& blocker_configuration : blocker_configurations<4096>(current_square, false))
				{
					attack_table[index] = rook_reachable_squares(current_square, blocker_configuration);
					++index;
				}
				for(const auto& blocker_configuration : blocker_configurations<512>(current_square, true))
				{
					attack_table[index] = bishop_reachable_squares(current_square, blocker_configuration);
					++index;
				}
			}
		}
		return attack_table;
	}
}



#endif // Bitboards_h_INCLUDED
