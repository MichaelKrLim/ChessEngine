#ifndef Move_h_INCLUDED
#define Move_h_INCLUDED

#include "Constants.h"
#include "Pieces.h"
#include "Position.h"

#include <cassert>
#include <cstdint>

namespace engine
{
	enum class Move_type
	{
		normal = 0,
		promotion = 1,
		en_passant = 2,
		castling = 3
	};

	class Move
	{
		public:

		constexpr Move() = default;
		constexpr Move(Position current_square, Position desired_square, Piece piece_type = Piece::knight) :
			Move(((static_cast<std::uint16_t>(piece_type)-static_cast<std::uint16_t>(Piece::knight))<<12) |
								 (to_index(current_square)<<6)                                          |
								  to_index(desired_square)) {};

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
			return static_cast<Piece>((move_data_ >> 11) & 0b11);
		}

		private:

		constexpr explicit Move(const std::uint16_t& move_data) : move_data_(move_data) {}

		// bits 0-5, destination square 
		// bits 6-11, origin square
		// bits 12-13, promotion piece type, (Piece::, from knight to queen)
		std::uint16_t move_data_{};

		friend std::ostream& operator<<(std::ostream& os, const Move& move);
	};

	inline std::ostream& operator<<(std::ostream& os, const Move& move)
	{
		const Position from_square{move.from_square()}, destination_square{move.destination_square()};
		return os << from_square << destination_square;
	}
}

#endif // Move_h_INCLUDED
