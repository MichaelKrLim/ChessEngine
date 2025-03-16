#ifndef Magic_util_h_INCLUDED
#define Magic_util_h_INCLUDED

#include "Bitboard.h"

namespace engine
{
	struct Magic_square
	{
		std::vector<Bitboard> attack_table;
		std::uint64_t mask;
		std::uint64_t magic;
		std::uint8_t shift;
	};

	constexpr std::uint64_t magic_hash(const Bitboard& key, const std::uint64_t& magic, int shift) 
	{
		return (key*magic) >> shift;
	}
}

#endif // Magic_util_h_INCLUDED
