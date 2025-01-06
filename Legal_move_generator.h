#ifndef Legal_move_generator_h_INCLUDED
#define Legal_move_generator_h_INCLUDED

#include "Board.h"
#include "Bitboard.h"
#include "Move.h"
#include "Pieces.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace engine 
{
	class Legal_move_generator
	{
		public:

		constexpr Legal_move_generator();
		const std::unordered_map<Piece, Bitboard> get(const Side_position&, const bool& is_white_to_move) const;
		
		private:

		constexpr void initialise_attack_table();
		constexpr void cast_magic();


		const Bitboard Legal_move_generator::pawn_legal_moves(const Bitboard& white_pawns, const Bitboard black_pawns, 
			const Bitboard& occupied_squares, const bool& is_white_to_move
		) const;
		const Bitboard king_legal_moves(const Position& square, const Bitboard& occupied_squares) const;
		const Bitboard knight_legal_moves(const Position& square, const Bitboard& occupied_squares) const;

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

		constexpr static std::array<Position, 8> knight_moves =
		{{
			Position{2, 1}, Position{2, -1}, Position{-2, 1}, Position{-2, -1},
			Position{1, 2}, Position{1, -2}, Position{-1, 2}, Position{-1, -2}
		}};
		constexpr static std::array<Position, 4> bishop_moves =
		{{
			Position{1, 1}, Position{1, -1},
			Position{-1, 1}, Position{-1, -1}
		}};
		constexpr static std::array<Position, 8> king_moves =
		{{
			Position{1, 0}, Position{-1, 0}, Position{0, 1}, Position{0, -1},
			Position{1, 1}, Position{-1, 1}, Position{1, -1},Position{-1, -1}
		}};
	};
}

#endif // Legal_move_generator_h_INCLUDED
