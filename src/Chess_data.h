#ifndef Chess_data_h_INCLUDED
#define Chess_data_h_INCLUDED

#include "Constants.h"
#include "Pieces.h"
#include "Position.h"

namespace engine
{
	namespace chess_data
	{
		constexpr static Side_map<Piece_map<int>> piece_values = []() constexpr
		{
			Side_map<Piece_map<int>> piece_values{};
			auto& white_values=piece_values[Side::white];
			auto& black_values=piece_values[Side::black];
			white_values[Piece::pawn]   = 100;
			white_values[Piece::knight] = 288;
			white_values[Piece::bishop] = 345;
			white_values[Piece::rook]   = 480;
			white_values[Piece::queen]  = 1077;
			for(const auto& piece : all_pieces)
				black_values[piece]=white_values[piece]*-1;
			return piece_values;
		}();

		using weightmap_type = Piece_map<std::array<int, board_size*board_size>>;
		constexpr static Side_map<weightmap_type> weightmaps = []() constexpr
		{
			Side_map<weightmap_type> weightmaps{};
			weightmaps[Side::white][Piece::pawn] =
			{
				0,  0,  0,  0,  0,  0,  0,  0,
				5, 10, 10,-20,-20, 10, 10,  5,
				5, -5,-10,  0,  0,-10, -5,  5,
				0,  0,  0, 20, 20,  0,  0,  0,
				5,  5, 10, 25, 25, 10,  5,  5,
				10, 10, 20, 30, 30, 20, 10, 10,
				50, 50, 50, 50, 50, 50, 50, 50,
				0,  0,  0,  0,  0,  0,  0,  0
			};
			weightmaps[Side::white][Piece::knight] =
			{
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50
			};
			weightmaps[Side::white][Piece::bishop] =
			{
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10,-10,-10,-10,-10,-20
			};
			weightmaps[Side::white][Piece::rook] =
			{
				0,  0,  5,  10, 10, 5,  0,  0,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				5,  10, 10, 10, 10, 10, 10, 5,
				0,  0,  0,  0,  0,  0,  0,  0,
			};
			weightmaps[Side::white][Piece::queen] =
			{
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-10,  5,  5,  5,  5,  5,  0,-10,
				0,  0,  5,  5,  5,  5,  0, -5,
				-5,  0,  5,  5,  5,  5,  0, -5,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
			};
			weightmaps[Side::white][Piece::king] =
			{
				20,  30,  10,  0,   0,   10,  30,  20,
				20,  20,  0,   0,   0,   0,   20,  20,
				-10, -20, -20, -20, -20, -20, -20, -10,
				-20, -30, -30, -40, -40, -30, -30, -20,
				-30, -40, -40, -50, -50, -40, -40, -30,
				-30, -40, -40, -50, -50, -40, -40, -30,
				-30, -40, -40, -50, -50, -40, -40, -30,
				-30, -40, -40, -50, -50, -40, -40, -30,
			};

			weightmaps[Side::black] = weightmaps[Side::white];
			for(auto& weightmap : weightmaps[Side::black])
			{
				for(std::uint8_t rank{0}; rank<board_size/2; ++rank)
				{
					for(std::uint8_t file{0}; file<board_size; ++file)
					{
						Position above_midpoint = Position{rank, file};
						Position below_midpoint = Position{static_cast<std::uint8_t>(board_size-1-rank), file};
						weightmap[to_index(above_midpoint)]*=-1;
						weightmap[to_index(below_midpoint)]*=-1;
						std::swap(weightmap[to_index(above_midpoint)], weightmap[to_index(below_midpoint)]);
					}
				}
			}
			return weightmaps;
		}();
	}
}

#endif
