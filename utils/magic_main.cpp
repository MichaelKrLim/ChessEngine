#include <array>
#include <fstream>

#include "Magic_generation_util.h"
#include "Magic_util.h"
#include "Pieces.h"
#include "Position.h"

using namespace engine;

int main()
{
	std::ofstream magic_header("magic_squares.h");
	std::array<Magic_square, 64> bishop_magic_squares{};
	magic_header << 
R"(#include <array>

#include "Bitboard.h"
#include "Magic_util.h"

const std::array<engine::Magic_square, 64> bishop_magic_squares_ = []() {
std::array<engine::Magic_square, 64> bishop_magic_squares{};

)";
	for(std::size_t rank{0}; rank<board_size; ++rank)
	{
		for(std::size_t file{0}; file<board_size; ++file)
		{
			const Position current_square = Position{rank, file};
			const auto current_index = to_index(current_square);
			bishop_magic_squares[current_index] = magic::find_magic(current_square, Piece::bishop);
			const auto& current_magic_square = bishop_magic_squares[current_index];
			magic_header << "bishop_magic_squares[" << current_index << "] = {{";
			for(const auto& bb : current_magic_square.attack_table)
			{
				magic_header << "Bitboard{" << static_cast<std::uint64_t>(bb) << "ULL}, ";
			}
			magic_header << "},\n"
			<< "Bitboard{" << static_cast<std::uint64_t>(current_magic_square.mask) << "ULL}, "
			<< current_magic_square.magic << "ULL, "
			<< static_cast<int>(current_magic_square.shift) << "u};\n";
		}
	}
	magic_header << " return bishop_magic_squares; }();\n";

	std::array<Magic_square, 64> rook_magic_squares{};
	magic_header << 
R"(
const std::array<engine::Magic_square, 64> rook_magic_squares_ = []() {
std::array<engine::Magic_square, 64> rook_magic_squares{};

)";
	for(std::size_t rank{0}; rank<board_size; ++rank)
	{
		for(std::size_t file{0}; file<board_size; ++file)
		{
			const Position current_square = Position{rank, file};
			const auto current_index = to_index(current_square);
			rook_magic_squares[to_index(current_square)] = magic::find_magic(current_square, Piece::rook);
			const auto& current_magic_square = rook_magic_squares[current_index];
			magic_header << "rook_magic_squares[" << current_index << "] = {{";
			for(const auto& bb : current_magic_square.attack_table)
			{
				magic_header << "Bitboard{" << static_cast<std::uint64_t>(bb) << "ULL}, ";
			}
			magic_header << "},\n"
			<< "Bitboard{" << static_cast<std::uint64_t>(current_magic_square.mask) << "ULL}, "
			<< current_magic_square.magic << "ULL, "
			<< static_cast<int>(current_magic_square.shift) << "u};\n";
		}
	}
	magic_header << " return rook_magic_squares; }();\n\n";

	magic_header << "const std::array<Bitboard, 64> king_mask = []() {\n" << "std::array<Bitboard, 64> king_mask{};\n";
	for(std::size_t board_index{0}; board_index<64; ++board_index)
	{
		Position position{board_index};
		magic_header << std::dec << "king_mask[" << board_index << "]=";
		magic_header << std::hex << "Bitboard{0x" << static_cast<std::uint64_t>(engine::magic::king_mask(position)) << "ULL};\n";
	}
	magic_header << "return king_mask; }();\n\n";
	
	magic_header << "const std::array<Bitboard, 64> knight_mask = []() {\n" << "std::array<Bitboard, 64> knight_mask{};\n";
	for(std::size_t board_index{0}; board_index<64; ++board_index)
	{
		Position position{board_index};
		magic_header << std::dec << "knight_mask[" << board_index << "]=";
		magic_header << std::hex << "Bitboard{0x" << static_cast<std::uint64_t>(engine::magic::knight_mask(position)) << "ULL};\n";
	}
	magic_header << "return knight_mask; }();\n\n";
}
