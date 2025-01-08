#ifndef Bitboards_h_INCLUDED
#define Bitboards_h_INCLUDED

#include "Board.h"
#include "Move.h"
#include "Pieces.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace engine 
{
	inline bool is_occupied(const std::uint64_t bitboard, const Position& square) { return bitboard & (1<<square.rank_*board_size+square.file_); }

	class Bitboards
	{
		public:

		constexpr Bitboards();
		[[nodiscard]] const std::unordered_map<Piece, std::uint64_t> get(const Side_position&, const bool& is_white_to_move) const;
		
		private:

		constexpr void initialise_attack_table();
		constexpr void cast_magic();

		const std::uint64_t pawn_legal_moves(const std::uint64_t& white_pawns, const std::uint64_t black_pawns,
			const std::uint64_t& occupied_squares, const bool& is_white_to_move
		) const;
		const std::uint64_t king_legal_moves(const Position& square, const std::uint64_t& occupied_squares) const;
		const std::uint64_t knight_legal_moves(const Position& square, const std::uint64_t& occupied_squares) const;

		constexpr const std::uint64_t rook_reachable_squares(const Position& square, const std::uint64_t& occupied_squares) const;
		constexpr const std::uint64_t bishop_reachable_squares(const Position& square, const std::uint64_t& occupied_squares) const;

		struct Magic_square
		{
			const std::vector<std::uint64_t>& attack_table;
			const std::uint64_t mask;
			const std::uint64_t magic;
			const int shift;
		};

		//constexpr static std::array<Magic_square, 64> bishop_magic_squares_; TO INITIALISE
		//constexpr static std::array<Magic_square, 64> rook_magic_squares_;
		//constexpr static std::array<std::uint64_t, 1408> attack_table_;

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

#endif // Bitboards_h_INCLUDED
