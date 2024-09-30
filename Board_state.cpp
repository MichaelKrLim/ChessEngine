#include "Board_state.h"

#include <algorithm>
#include <iostream>

std::array<std::array<int, Board_state::size*Board_state::size>, 6> Board_state::white_weightmaps =
{{
	{{
		//Piece::pawn
		 0,  0,  0,  0,  0,  0,  0,  0,
		 5, 10, 10,-20,-20, 10, 10,  5,
		 5, -5,-10,  0,  0,-10, -5,  5,
		 0,  0,  0, 20, 20,  0,  0,  0,
		 5,  5, 10, 25, 25, 10,  5,  5,
		 10, 10, 20, 30, 30, 20, 10, 10,
		 50, 50, 50, 50, 50, 50, 50, 50,
		 0,  0,  0,  0,  0,  0,  0,  0
	}},
	{{
		//Piece::knight
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-30,  5, 10, 15, 15, 10,  5,-30,
		-30,  0, 15, 20, 20, 15,  0,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  0, 10, 15, 15, 10,  0,-30,
		-40,-20,  0,  0,  0,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50
	}},
	{{
		//Piece::bishop,
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  5,  0,  0,  0,  0,  5,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10,  5,  5, 10, 10,  5,  5,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-20,-10,-10,-10,-10,-10,-10,-20
	}},
	{{
		//Piece::rook,
		 0,  0,  5,  10, 10, 5,  0,  0,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		 5,  10, 10, 10, 10, 10, 10, 5,
		 0,  0,  0,  0,  0,  0,  0,  0,
	}},
	{{
		//Piece::queen,
		-20,-10,-10, -5, -5,-10,-10,-20
		-10,  0,  5,  0,  0,  0,  0,-10,
		-10,  5,  5,  5,  5,  5,  0,-10,
		 0,  0,  5,  5,  5,  5,  0, -5,
		-5,  0,  5,  5,  5,  5,  0, -5,
		-10,  0,  5,  5,  5,  5,  0,-10,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20,

	}},
	{{
		//Piece::king,
		 20,  30,  10,  0,   0,   10,  30,  20,
		 20,  20,  0,   0,   0,   0,   20,  20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
	}},
}};

std::array<int, 6> Board_state::piece_values =
{
	//Piece::pawn
	1,
	//Piece::knight
	3,
	//Piece::bishop
	3,
	//Piece::rook
	5, 
	//Piece::queen
	9
};

Board_state::weightmap_type Board_state::black_weightmaps = Board_state::generate_black_weightmap();

Board_state::weightmap_type Board_state::generate_black_weightmap()
{
	weightmap_type white_weightmaps_copy = white_weightmaps;
	for(auto& weightmap : white_weightmaps_copy)
	{
		for(std::uint8_t rank{0}; rank<size/2; ++rank)
		{
			for(std::uint8_t file{0}; file<size; ++file)
			{
				Position ubove_midpoint = {rank, file};
				Position below_midpoint = {static_cast<std::uint8_t>(size-rank), file};
				std::swap(weightmap[to_index(ubove_midpoint)], weightmap[to_index(below_midpoint)]);
			}
		}
	}
	return white_weightmaps_copy;
}

void Board_state::output_weights()
{
	std::cout << "white: \n";
	for(const auto& weightmap : white_weightmaps)
	{
		for(const auto& val : weightmap)
		{
			std::cout << val << ", ";
		}
		std::cout << "\n";
	}
	std::cout << "black: \n";
	for(const auto& weightmap : black_weightmaps)
	{
		for(const auto& val : weightmap)
		{
			std::cout << val << ", ";
		}
		std::cout << "\n";
	}
}


std::size_t Board_state::to_index(Position position)
{
	return size*position.rank + position.file;
}

double Board_state::white_material_value() const
{
	//TODO
}

void Board_state::init_black_weightmaps()
{
	//TODO
}

double Board_state::black_material_value() const
{
	//TODO
}

bool Board_state::white_is_mated() const
{
	//TODO
}

bool Board_state::black_is_mated() const
{
	//TODO
}

bool Board_state::is_draw() const
{
	//TODO
}

double Board_state::evaluate() const
{
	//TODO
}
