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
	public:
		
	const static int size = 8;

	[[nodiscard]] const bool white_is_mated() const;
	[[nodiscard]] const bool black_is_mated() const;
	[[nodiscard]] const bool is_draw() const;
	[[nodiscard]] const double evaluate() const;
	
	private:
	
	std::unordered_map<Piece, uint64_t> pieces;
	static std::unordered_map<Piece, int> piece_values;
	static std::unordered_map<Piece, std::array<std::array<int, size-1>, size-1>> black_weightmaps;
	static std::unordered_map<Piece, std::array<std::array<int, size-1>, size-1>> white_weightmaps;
	
	[[nodiscard]] const double white_material_value() const;
	[[nodiscard]] const double black_material_value() const;
	void init_black_weightmaps();
};

#endif // Board_state_h_INCLUDED
