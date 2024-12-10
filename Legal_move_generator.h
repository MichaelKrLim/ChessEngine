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

		constexpr Legal_move_generator();
		std::vector<Move> get();
		
		private:

		constexpr void initialise_attack_table();
		constexpr void cast_magic();

		std::uint64_t knights_reachable_squares(const Position& square, const uint64_t& occupied_squares);
		std::uint64_t rooks_reachable_squares(const Position& square, const uint64_t& occupied_squares);
		constexpr std::uint64_t bishops_reachable_squares(const Position& square, const uint64_t& occupied_squares);
		constexpr void explore_diagonal(const Position& bishop_square, const Position& diagonal_offset,
			const uint64_t occupied_squares,
			std::uint64_t& valid_moves, 
			const Position& origional_bishop_square
		);

		constexpr struct Magic_square
		{
			const std::vector<std::uint64_t>& attack_table;
			const std::uint64_t mask;
			const std::uint64_t magic;
			const int shift;
		};

		constexpr static std::array<Magic_square, 64> bishops_magic_squares_;
		constexpr static std::array<Magic_square, 64> rooks_magic_squares_;

		constexpr static std::vector<std::uint_64_t> attack_table_{};

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
