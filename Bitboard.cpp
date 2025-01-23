#include "Bitboard.h"
#include "Constants.h"

using namespace	engine;

// official-stockfish (2024). Stockfish/src/bitboard.cpp at	9766db8139ce8815110c15bdde8381d0564a63fa · official-stockfish/Stockfish. [online] GitHub. Available	at:	https://github.com/official-stockfish/Stockfish/blob/9766db8139ce8815110c15bdde8381d0564a63fa/src/bitboard.cpp [Accessed 9 Jan.	2025].
std::string	Bitboard::pretty_string() const	
{
	std::string s = "+---+---+---+---+---+---+---+---+\n";
	for(int r = 7;	r >= 0;	--r)
	{
		for(std::size_t f = 0; f <= 7; ++f)
			s += (data_ & (1ULL << (f+r*board_size)))? "| X " : "|   ";

		s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
	}
	s += "  a   b   c   d   e   f   g   h\n";

	return s;
}

bool Bitboard::is_occupied(const Position& square) const
{
	return data_ & (1<<square.rank_*board_size+square.file_); 
}

bool Bitboard::is_occupied(const std::uint64_t& position) const
{
	return data_ & (1<<position);
}

void Bitboard::hash(const int& magic)
{
	data_ *= magic;
}
