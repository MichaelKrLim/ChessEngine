#ifndef Constants_h_INCLUDED
#define Constants_h_INCLUDED

#include <array>
#include <cstdint>

namespace engine 
{
	enum class Side
	{
		white, black
	};

	constexpr std::uint8_t board_size = 8,
	white_en_passant_target_rank = 3,
	black_en_passant_target_rank = 6,
	max_legal_moves = 218;

	constexpr std::uint64_t file_a{0x101010101010101}, file_h{0x8080808080808080};
	
	constexpr std::array<char, board_size> to_algebraic_file = 
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
	};
}

#endif // Constants_h_INCLUDED
