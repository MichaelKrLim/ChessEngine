#include "Engine.h"
#include "Position.h"
#include "Pieces.h"

#include <array>
#include <iostream>

using namespace engine;

std::array<std::array<int, board_size*board_size>, 6> Engine::white_weightmaps =
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

std::array<int, 6> Engine::piece_values =
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

Engine::weightmap_type Engine::black_weightmaps = Engine::generate_black_weightmap();

Engine::weightmap_type Engine::generate_black_weightmap()
{
	weightmap_type white_weightmaps_copy = white_weightmaps;
	for(auto& weightmap : white_weightmaps_copy)
	{
		for(std::uint8_t rank{0}; rank<board_size/2; ++rank)
		{
			for(std::uint8_t file{0}; file<board_size; ++file)
			{
				Position above_midpoint = Position{rank, file};
				Position below_midpoint = Position{static_cast<std::uint8_t>(board_size-1-rank), file};
				std::swap(weightmap[to_index(above_midpoint)], weightmap[to_index(below_midpoint)]);
			}
		}
	}
	return white_weightmaps_copy;
}

void Engine::output_weights() const
{
	std::cout << "white: \n";
	for(const auto& weightmap : white_weightmaps)
	{
		for(int i{0}; i<weightmap.size(); ++i)
		{
			if(i%board_size==0)
				std::cout << "\n";
			std::cout << weightmap[i] << ", ";
		}
		std::cout << "\n";
	}
	std::cout << "black: \n";
	for(const auto& weightmap : black_weightmaps)
	{
		for(int i{0}; i<weightmap.size(); ++i)
		{
			if(i%board_size==0)
				std::cout << "\n";
			std::cout << weightmap[i] << ", ";
		}
		std::cout << "\n";
	}
}

double Engine::white_material_value() const
{
	//TODO
}

double Engine::black_material_value() const
{
	//TODO
}
