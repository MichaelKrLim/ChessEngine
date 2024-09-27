#include "Board_state.h"

#include <algorithm>

std::unordered_map<Piece, std::array<std::array<int, size-1>, size-1>> Board_state::white_weightmaps =
{
	{
		Piece::pawn,
		 0,  0,  0,  0,  0,  0,  0,  0,
		 5, 10, 10,-20,-20, 10, 10,  5,
		 5, -5,-10,  0,  0,-10, -5,  5,
		 0,  0,  0, 20, 20,  0,  0,  0,
		 5,  5, 10, 25, 25, 10,  5,  5,
		 10, 10, 20, 30, 30, 20, 10, 10,
		 50, 50, 50, 50, 50, 50, 50, 50,
		 0,  0,  0,  0,  0,  0,  0,  0
	},
	{
		Piece::knight,
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-30,  5, 10, 15, 15, 10,  5,-30,
		-30,  0, 15, 20, 20, 15,  0,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  0, 10, 15, 15, 10,  0,-30,
		-40,-20,  0,  0,  0,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50
	},
	{
		Piece::bishop,
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  5,  0,  0,  0,  0,  5,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10,  5,  5, 10, 10,  5,  5,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-20,-10,-10,-10,-10,-10,-10,-20
	},
	{
		Piece::rook,
		 0,  0,  5,  10, 10, 5,  0,  0,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		 5,  10, 10, 10, 10, 10, 10, 5,
		 0,  0,  0,  0,  0,  0,  0,  0,

	},
	{
		Piece::queen,
		-20,-10,-10, -5, -5,-10,-10,-20
		-10,  0,  5,  0,  0,  0,  0,-10,
		-10,  5,  5,  5,  5,  5,  0,-10,
		 0,  0,  5,  5,  5,  5,  0, -5,
		-5,  0,  5,  5,  5,  5,  0, -5,
		-10,  0,  5,  5,  5,  5,  0,-10,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20,

	},
	{
		Piece::king,
		 20,  30,  10,  0,   0,   10,  30,  20,
		 20,  20,  0,   0,   0,   0,   20,  20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
	},
};
std::unordered_map<Piece, int> Board_state::piece_values =
{
	{Piece::pawn, 1}, 
	{Piece::knight, 3},
	{Piece::bishop, 3}, 
	{Piece::rook, 5}, 
	{Piece::queen, 9}
};

auto white_weightmaps_copy
for(const auto& keyed_weightmap : white_weightmaps_copy)
{
	auto [Piece, weightmap] = keyed_weightmap;
	std::ranges::reverse(weightmap_weightmap);
}
std::unordered_map<Piece, std::array<std::array<int, size>, size>> Board_state::black_weightmaps = white_weightmaps_copy;
	
	const double Board_state::white_material_value() const
	{
		//TODO
	}
		
	void Board_state::init_black_weightmaps()
	{

	}
	
	const double Board_state::black_material_value() const
	{
		//TODO
	}
	
	const bool Board_state::white_is_mated() const
	{
		//TODO
	}
	
	const bool Board_state::black_is_mated() const
	{
		//TODO
	}
	
	const bool Board_state::is_draw() const
	{
		//TODO
	}
	
	const double Board_state::evaluate() const
	{
		//TODO
	}
