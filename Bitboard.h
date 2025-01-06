#ifndef Bitboard_h_INCLUDED
#define Bitboard_h_INCLUDED

#include <cstdint>

#include "Constants.h"

class Bitboard : std::uint64_t
{
    public:

    bool is_occupied(const Position& square)
    {
        return *this & (1<<square.rank_*board_size+square.file_);
    }
}

#endif // Bitboard_h_INCLUDED