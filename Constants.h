#ifndef Constants_h_INCLUDED
#define Constants_h_INCLUDED

#include <cstdint>

namespace engine 
{
	enum class Side
	{
		white, black
	};

	const static std::uint8_t board_size = 8;
}

#endif // Constants_h_INCLUDED
