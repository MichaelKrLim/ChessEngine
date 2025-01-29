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

		constexpr void initialise_attack_table();
		constexpr void cast_magic();

		const moves_type pawn_legal_moves(const Bitboard& pawn_bb, const Bitboard& occupied_squares, const Side& active_player) const;
		const moves_type king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares) const;
		const moves_type knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares) const;

		constexpr const Bitboard rook_reachable_squares(const Position& square, const Bitboard& occupied_squares) const;
		constexpr const Bitboard bishop_reachable_squares(const Position& square, const Bitboard& occupied_squares) const;

		struct Magic_square
		{
			const std::vector<Bitboard>& attack_table;
			const Bitboard mask;
			const Bitboard magic;
			const int shift;
		};

		constexpr static std::array<Magic_square, 64> bishop_magic_squares_;
		constexpr static std::array<Magic_square, 64> rook_magic_squares_;
		constexpr static std::array<Bitboard, 1408> attack_table_;

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
}

#endif // Bitboards_h_INCLUDED
