#ifndef Constants_h_INCLUDED
#define Constants_h_INCLUDED

#include <cstdint>

namespace engine 
{
	enum class Side
	{
		white, black
	};

	constexpr std::uint8_t board_size = 8;
	constexpr std::uint64_t file_a = 0x0101010101010101ULL;
	constexpr std::uint64_t file_h = 0x8080808080808080ULL;
}

#endif // Constants_h_INCLUDED
