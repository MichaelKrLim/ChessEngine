#ifndef Position_h_INCLUDED
#define Position_h_INCLUDED

#include "Square.h"
#include "Constants.h"

#include <cstdint>

namespace engine
{
	struct Position
	{
		explicit inline Position(const Square& square)
		{
			*this=to_position(static_cast<size_t>(square));
		}

		std::uint8_t rank, file;
		std::uint8_t maximal_mask = 0b11111111;
	};

	inline std::size_t to_index(const Position& position)
	{
		return engine::board_size*position.rank+position.file;
	}

	inline Position to_position(const std::size_t& index)
	{
		return Position{index/engine::board_size, index%engine::board_size};
	}
	
	inline bool is_on_board(const std::size_t& index)
	{
		if(position.rank>=0 && position.rank <= 8 && position.file >=0 && position.file <= 8)
			return true;
		return false;
	}

	inline bool is_occupied(const std::size_t& board_index, const uint64_t occupied_squares)
	{
		std::size_t board_index = static_cast<std::size_t>(square);
		if(!is_on_board(board_index))
			return false;

		bool square_is_occupied = (1 << board_index & occupied_squares);
		if(!square_is_occupied)
			return true;
		return false
	}

	inline bool is_occupied(const Square& square, const uint64_t& occupied_squares)
	{
		std::size_t board_index = static_cast<std::size_t>(square);
		is_occupied(board_index, occupied_squares);
	}
}

#endif // Position_h_INCLUDED
