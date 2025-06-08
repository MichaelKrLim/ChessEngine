#ifndef Move_h_INCLUDED
#define Move_h_INCLUDED

#include "Constants.h"
#include "Pieces.h"
#include "Position.h"

#include <cassert>
#include <cstdint>
#include <unordered_map>

namespace engine
{
	enum class Is_capture { yes, no };

	class Move
	{
		public:

		constexpr Move() = default;
		constexpr Move(Position current_square, Position desired_square, Is_capture is_capture = Is_capture::no)
		{
			move_data_ |= (to_index(current_square)<<6) | to_index(desired_square);
			if(is_capture == Is_capture::yes)
				move_data_ |= (1U << 15);
		};
		constexpr Move(Position current_square, Position desired_square, Piece piece_type, Is_capture is_capture = Is_capture::no) : Move(current_square, desired_square, is_capture)
		{
			move_data_ |= (1U<<14) | ((static_cast<std::uint16_t>(piece_type)-static_cast<std::uint16_t>(Piece::knight))<<12);
		};

		constexpr auto operator<=>(const Move&) const = default;

		constexpr Position from_square() const
		{
			return Position((move_data_>>6) & (board_size*board_size-1));
		}
		constexpr Position destination_square() const
		{
			return Position(move_data_ & (board_size*board_size-1));
		}
		constexpr Piece promotion_piece() const
		{
			return static_cast<Piece>(((move_data_ >> 12) & 0b11) + static_cast<std::uint8_t>(Piece::knight));
		}
		constexpr bool is_promotion() const
		{
			return move_data_ & (1U << 14);
		}
		constexpr bool is_capture() const
		{
			return move_data_ & (1U << 15);
		}

		private:

		constexpr explicit Move(const std::uint16_t& move_data) : move_data_(move_data) {}

		// bits 0-5, destination square 
		// bits 6-11, origin square
		// bits 12-13, promotion piece type, (Piece::, from knight to queen)
		// bit 14 promotion flag
		std::uint16_t move_data_{0};

		friend std::ostream& operator<<(std::ostream& os, const Move& move);
	};

	inline std::ostream& operator<<(std::ostream& os, const Move& move)
	{
		const Position from_square{move.from_square()}, destination_square{move.destination_square()};
		const Piece promotion_piece{move.promotion_piece()};
		os << from_square << destination_square;
		constexpr Piece_map<char> to_algebraic_piece = {' ', ' ', 'n', 'b', 'r', 'q'};
		if(move.is_promotion())
			return os << to_algebraic_piece[promotion_piece];
		return os;
	}

	inline std::istream& operator>>(std::istream& is, Move& move)
	{
		std::string move_string;
		is>>move_string;
		static std::unordered_map<char, Piece> to_piece{{'n', Piece::knight}, {'q', Piece::queen}, {'b', Piece::bishop}, {'r', Piece::rook}};
		if(move_string.size() == 4)
			move = Move{algebraic_to_position(move_string.substr(0, 2)), algebraic_to_position(move_string.substr(2, 2))};
		else if(move_string.size() == 5)
			move = Move{algebraic_to_position(move_string.substr(0, 2)), algebraic_to_position(move_string.substr(2, 2)), to_piece[move_string.back()]};
		else
			is.setstate(std::ios::failbit);
		return is;
	}
}

#endif // Move_h_INCLUDED
