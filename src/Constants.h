#ifndef Constants_h_INCLUDED
#define Constants_h_INCLUDED

#include "Enum_map.h"

#include <array>
#include <cstdint>

namespace engine 
{
	enum class Side
	{
		white, black, size
	};

	template <typename Mapped_type>
	using Side_map = Enum_map_from_size<Side, Mapped_type>;

	constexpr auto all_sides = {Side::white, Side::black};

	inline Side operator!(const Side& side) noexcept
	{
		return side == Side::white? Side::black : Side::white;
	}

	constexpr std::uint8_t board_size = 8,
	number_of_pieces = 6,
	white_en_passant_target_rank = 3,
	black_en_passant_target_rank = 6,
	max_legal_moves = 218,
	king_max_adjacent_squares = 6;

	constexpr std::uint64_t file_a{0x101010101010101}, file_h{0x8080808080808080};
	
	constexpr std::array<char, board_size> to_algebraic_file = 
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
	};
}

#endif // Constants_h_INCLUDED
