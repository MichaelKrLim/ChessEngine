#ifndef Position_h_INCLUDED
#define Position_h_INCLUDED

#include "Constants.h"

#include <cassert>
#include <type_traits>

namespace engine
{
	struct Bitboard;
	struct Position
	{
		constexpr bool operator==(const Position& rhs) const { return rank_ == rhs.rank_ && file_ == rhs.file_; }
		constexpr Position operator+(const Position& to_add) const { return Position{rank_ + to_add.rank_, file_ + to_add.file_}; }
		
		constexpr Position() = default;
		explicit constexpr Position(const std::size_t& board_index)
		{
			rank_ = board_index/board_size;
			file_ = board_index%board_size;
		}
		template <typename T, typename L, typename = std::enable_if_t<std::is_integral<T>::value>, typename = std::enable_if_t<std::is_integral<L>::value>>
		explicit constexpr Position(T rank, L file) : rank_(static_cast<int>(rank)), file_(static_cast<int>(file)) {}

		std::uint8_t rank_{}, file_{};
	};

	constexpr std::size_t to_index(const Position& position)
	{
		return board_size*position.rank_+position.file_;
	}

	constexpr bool is_on_board(const Position& position)
	{
		return position.rank_<8 && position.file_<8;
	}
}

#endif // Position_h_INCLUDED
