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
R"(
std::array<Magic_square, 64> bishop_magic_squares_ = []() {
std::array<Magic_square, 64> bishop_magic_squares{};

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
			<< current_magic_square.mask << "ULL, "
			<< current_magic_square.magic << "ULL, "
			<< static_cast<int>(current_magic_square.shift) << "};\n";
		}
	}
	magic_header << " return bishop_magic_squares; }();\n";

	std::array<Magic_square, 64> rook_magic_squares{};
	magic_header << 
R"(
std::array<Magic_square, 64> rook_magic_squares_ = []() {
std::array<Magic_square, 64> rook_magic_squares{};

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
			<< current_magic_square.mask << "ULL, "
			<< current_magic_square.magic << "ULL, "
			<< static_cast<int>(current_magic_square.shift) << "};\n";
		}
	}
	magic_header << " return rook_magic_squares; }();\n";
}
