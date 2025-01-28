#ifndef Position_h_INCLUDED
#define Position_h_INCLUDED

#include "Bitboard.h"
#include "Constants.h"

#include <cassert>
#include <cstdint>

namespace engine
{
	struct Position
	{
		Position operator+(const Position& to_add) const
		{
			return Position{rank_ + to_add.rank_, file_ + to_add.file_};
		}
		
		constexpr Position() = default;
		explicit constexpr Position(const std::size_t& board_index)
		{
			rank_ = board_index/board_size;
			file_ = board_index%board_size;
		}
		explicit constexpr Position(const int& rank, const int& file) : rank_(rank), file_(file) {}

		std::uint8_t rank_{}, file_{};
	};

	inline constexpr std::size_t to_index(const Position& position)
	{
		return board_size*position.rank_+position.file_;
	}

	inline constexpr bool is_on_board(const Position& position)
	{
		return position.rank_<8 && position.file_<8;
	}

	inline constexpr bool is_valid_destination(const Position& board_index, const Bitboard& occupied_squares)
	{
		if(!is_on_board(board_index))
			return false;

		bool square_is_occupied = (occupied_squares & 1 << to_index(Position(board_index))) > 0;
		return square_is_occupied;
	}
}

#endif // Position_h_INCLUDED
