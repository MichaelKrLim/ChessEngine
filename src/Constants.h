#ifndef Constants_h_INCLUDED
#define Constants_h_INCLUDED

#include "Enum_map.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <ostream>
#include <string_view>
#include <thread>

namespace engine 
{
	enum class Side
	{
		white, black, size
	};

	template <typename Mapped_type>
	using Side_map = Enum_map_from_size<Side, Mapped_type>;

	constexpr auto all_sides = {Side::white, Side::black};

	inline Side other_side(const Side& side) noexcept
	{
		return side == Side::white? Side::black : Side::white;
	}

	inline std::ostream& operator<<(std::ostream& os, const Side& side)
	{
		if(side == Side::white) os << "white";
		if(side == Side::black) os << "black";
		return os;
	}

	constexpr std::uint_fast8_t board_size = 8,
	number_of_pieces = 6,
	white_en_passant_target_rank = 3,
	black_en_passant_target_rank = 6,
	max_legal_moves = 218,
	max_depth = 64,
	king_max_adjacent_squares = 6;

	constexpr unsigned default_table_size{64};

	constexpr std::string_view name{"cpp_engine"}, author{"Michael Lim"}, starting_fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

	constexpr std::uint64_t file_a{0x101010101010101}, file_h{0x8080808080808080};
	
	constexpr std::array<char, board_size> to_algebraic_file = 
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
	};

	inline unsigned optimal_number_of_threads{4 /*std::max(unsigned{4}, std::thread::hardware_concurrency())*/};
} // namespace engine

namespace uci
{
	constexpr unsigned max_table_size{33554432};
	constexpr std::chrono::milliseconds max_move_overhead{5000};
} // namespace uci

#endif // Constants_h_INCLUDED
