#include <iostream>
#include <unordered_map>

enum class Piece
{
	pawn, knight, bishop, rook, queen, king;
};

class Board_state
{
	private:
	std::unordered_map<Piece, uint64_t> pieces;
	satic unordered_map<Piece, int> piece_values
	{
		{Piece::pawn, 1}, 
		{Piece::knight, 3},
		{Piece::bishop, 3}, 
		{Piece::rook, 5}, 
		{Piece::queen, 9}
	};

	[[nodiscard]] const double white_material_value() const;
	[[nodiscard]] const double black_material_value() const;
	
	public:
	const int size = 64;

	[[nodiscard]] const bool white_is_mated() const;
	[[nodiscard]] const bool black_is_mated() const;
	[[nodiscard]] const bool is_draw() const;
	[[nodiscard]] const double evaluate() const;
};

int main()
{
	
}
