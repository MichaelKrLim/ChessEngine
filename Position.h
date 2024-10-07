#ifndef Position_h_INCLUDED
#define Position_h_INCLUDED

#include "Constants.h"

#include <cstdint>

struct Position
{
	std::uint8_t rank, file;
};

inline std::size_t to_index(Position position)
{
	return engine::board_size*position.rank + position.file;
}

#endif // Position_h_INCLUDED
