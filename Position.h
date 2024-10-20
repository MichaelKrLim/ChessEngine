#ifndef Position_h_INCLUDED
#define Position_h_INCLUDED

#include "Square.h"
#include "Constants.h"


#include <cassert>
#include <cstdint>
#include <concepts>

namespace engine
{
	struct Position;	
	Position to_position(const std::size_t& index);
	
	struct Position
	{
		Position() = default;
		explicit inline Position(const Square& square)
		{
			*this=to_position(static_cast<std::size_t>(square));
		}
		explicit inline Position(const int& board_index)
		{
			const auto [rank, file] = to_position(board_index);
			rank_ = rank;
			file_ = file;
		}
		explicit inline Position(const int& rank, const int& file) : rank_(rank), file_(file) {}

		std::uint8_t rank_, file_;
		const static std::uint8_t maximal_mask = 0b11111111;
	};

	inline std::size_t to_index(const Position& position)
	{
		return board_size*position.rank_+position.file_;
	}

	template <typename index_type>
	inline bool is_on_board(const index_type& board_index) requires
		std::same_as<index_type, std::size_t> ||
		std::same_as<index_type, Position>
	{
		Position position{board_index};
		if(position.rank_>=0 && position.rank_<=8 && position.file_>=0 && position.file_<=8)
			return true;
		return false;
	}

	template <typename index_type>
	inline bool is_valid_destination(const index_type& board_index, const uint64_t& occupied_squares) requires
		std::same_as<index_type, std::size_t> ||
		std::same_as<index_type, Position> || 
		std::same_as<index_type, Square>
	{
		if(!is_on_board(Position{board_index}))
			return false;

		bool square_is_occupied = (1 << to_index(Position{board_index}) & occupied_squares);
		return square_is_occupied;
	}

	template<typename index_type>
	inline Position to_position(const index_type& index) requires
		std::same_as<index_type, std::size_t> ||
		std::same_as<index_type, Square>
	{
		index = static_cast<std::size_t>(index);
		return Position{index/board_size, index%board_size};
	}

	template<typename index_type>
	inline std::size_t to_file_index(const index_type& board_index) requires
		std::same_as<index_type, std::size_t> ||
		std::same_as<index_type, Square>
	{
		board_index = static_cast<std::size_t>(board_index);
		return board_index%board_size;
	}

	template<typename index_type>
	inline std::size_t to_rank_index(const index_type& board_index) requires
		std::same_as<index_type, std::size_t> ||
		std::same_as<index_type, Square>
	{
		board_index = static_cast<std::size_t>(board_index);
		return board_index/board_size;
	}
}

#endif // Position_h_INCLUDED
