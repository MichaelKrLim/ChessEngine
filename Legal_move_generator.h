#ifndef Legal_move_generator_h_INCLUDED
#define Legal_move_generator_h_INCLUDED

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

		Legal_move_generator();

		private:

		void initialise_attack_table();
		void cast_magic();

		std::vector<Move> knight_reachable_squares(const Position& square, const uint64_t& occupied_squares);
		std::vector<Move> rook_reachable_squares(const Position& square, const uint64_t& occupied_squares);
		std::vector<Move> bishop_reachable_squares(const Position& square, const uint64_t& occupied_squares);
		void explore_diagonal(const Position& bishop_square, const Position& diagonal_offset,
		const uint64_t occupied_squares, std::vector<Move>& valid_moves, const Position& origional_bishop_square);

		uint64_t reachable_squares(const Position& square, const Piece& piece, const uint64_t& occupied_squares);

		struct Magic_square
		{
			const std::vector<std::uint64_t>& attack_table;
			const std::uint64_t mask;
			const std::uint64_t magic;
			const int shift;
		};

		static std::array<Magic_square, 64> bishops_magic_squares_;
		static std::array<Magic_square, 64> rooks_magic_squares_;

		static std::vector<std::uint_64_t> attack_table_{};

		constexpr static std::array<std::array<int, 2>, 8> knight_moves =
		{{
			{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
			{1, 2}, {1, -2}, {-1, 2}, {-1, -2}
		}};
		constexpr static std::array<Position, 8> bishop_moves =
		{{
			Position{1, 1}, Position{1, -1},
			Position{-1, 1}, Position{-1, -1}
		}};
	};
}

#endif // Legal_move_generator_h_INCLUDED
