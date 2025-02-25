#ifndef Bitboards_h_INCLUDED
#define Bitboards_h_INCLUDED

#include "Board.h"
#include "Pieces.h"

#include <array>

namespace std 
{
	template <>
	struct hash<engine::Position>
	{
		size_t operator()(const engine::Position& p) const
		{
			size_t h1 = std::hash<int>{}(p.rank_);
			size_t h2 = std::hash<int>{}(p.file_);
			return h1 ^ (h2 << 1);
		}
	};
}

namespace engine
{
	using moves_type = std::unordered_map<Position, std::vector<Position>>;
	const std::array<moves_type, number_of_piece_types> legal_moves(const Board& board);
	void initialise_move_generator();
}

#endif // Bitboards_h_INCLUDED
