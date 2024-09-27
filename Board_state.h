#ifndef Board_state_h_INCLUDED
#define Board_state_h_INCLUDED

#include <unordered_map>
#include <array>
#include <cstdint>

enum class Piece
{
	pawn, knight, bishop, rook, queen, king
};

class Board_state
{
	private:
	std::unordered_map<Piece, uint64_t> pieces;
	static std::unordered_map<Piece, int> piece_values =
	{
		{Piece::pawn, 1}, 
		{Piece::knight, 3},
		{Piece::bishop, 3}, 
		{Piece::rook, 5}, 
		{Piece::queen, 9}
	};
	static std::unordered_map<Piece, std::array<std::array<Piece, size>, 5> black_weightmaps;
	static std::unordered_map<Piece, std::array<std::array<Piece, size>, 5> white_weightmaps = 
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

	[[nodiscard]] const double white_material_value() const;
	[[nodiscard]] const double black_material_value() const;
	void init_black_weightmaps();

	public:
	const int size = 64;

	[[nodiscard]] const bool white_is_mated() const;
	[[nodiscard]] const bool black_is_mated() const;
	[[nodiscard]] const bool is_draw() const;
	[[nodiscard]] const double evaluate() const;

	std::unordered_map<Piece, int>& white_weightmaps() { return white_weightmaps; }
	std::unordered_map<Piece, int>& black_weightmaps() { return black_weightmaps; }
	std::unordered_map<Piece, int>& piece_values() { return piece_values; }
};

#endif // Board_state_h_INCLUDED
