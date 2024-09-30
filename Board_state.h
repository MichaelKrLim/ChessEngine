#ifndef Board_state_h_INCLUDED
#define Board_state_h_INCLUDED

#include <unordered_map>
#include <array>
#include <cstdint>

enum class Piece : std::uint8_t
{
	pawn = 0, knight = 1, bishop = 2, rook = 3, queen = 4, king = 5
};

struct Position
{
	std::uint8_t rank, file;
};

class Board_state
{
	public:
		
	const static uint8_t size = 8;

	[[nodiscard]] bool white_is_mated() const;
	[[nodiscard]] bool black_is_mated() const;
	[[nodiscard]] bool is_draw() const;
	[[nodiscard]] double evaluate() const;
	void output_weights();
	
	private:
	
	std::unordered_map<Piece, std::uint64_t> white_pieces;
	std::unordered_map<Piece, std::uint64_t> black_pieces;

	static std::array<int, 6> piece_values;
	using weightmap_type = std::array<std::array<int, size*size>, 6>;
	static weightmap_type white_weightmaps;
	static weightmap_type black_weightmaps;
	
	[[nodiscard]] double white_material_value() const;
	[[nodiscard]] double black_material_value() const;
	[[nodiscard]] static weightmap_type generate_black_weightmap();
	[[nodiscard]] static std::size_t to_index(Position position);
	void init_black_weightmaps();
};

#endif // Board_state_h_INCLUDED
