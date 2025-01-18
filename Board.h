#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Constants.h"
#include "FEN.h"
#include "Pieces.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>

#include "Bitboard.h"

namespace engine
{
	struct Side_position
	{
		std::array<Bitboard, 6> pieces{};
		bool can_castle_left, can_castle_right;
		Bitboard occupied_squares{};
	};

	struct Board
	{
		explicit inline Board(const std::string& FEN_string, const FEN& fen)
		{
			*this = fen.from_string(FEN_string);
		}
		inline Board() = default;

		Side_position black{};
		Side_position white{};

		std::uint64_t occupied_squares{};
	};
}

#endif // Board_h_INCLUDED
