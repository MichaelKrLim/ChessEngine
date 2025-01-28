#ifndef Move_h_INCLUDED
#define Move_h_INCLUDED

#include "Constants.h"
#include "Pieces.h"
#include "Position.h"

#include <cassert>
#include <cstdint>

enum class Move_type
{
	normal,
	promotion = 1,
	en_passant = 2,
	castling = 3
};

namespace engine
{
	class Move
	{
		public:

		constexpr Move() = default;
		constexpr Move(const Position& current_square, const Position& desired_square) :
			move_data_((to_index(current_square)<<6) | to_index(desired_square)) {}

		static constexpr Move make(Position current_square, Position desired_square, Move_type move_type, Piece piece_type = Piece::knight)
		{
			return Move(static_cast<std::uint16_t>(move_type)<<14|
					  ((static_cast<std::uint16_t>(piece_type)-static_cast<std::uint16_t>(Piece::knight))<<12)|
										 (to_index(current_square)<<6)|
										  to_index(desired_square));
		}
		constexpr Position from_square() const
		{
			return Position((move_data_>>6)&board_size*board_size-1);
		}
		constexpr Position destination_square() const
		{
			return Position(move_data_&board_size*board_size-1);
		}

		private:

		constexpr explicit Move(const std::uint16_t& move_data) : move_data_(move_data) {}

		// bits 0-5, destination square 
		// bits 6-11, origin square
		// bits 12-13, promotion piece type, (Piece::, from knight to queen)
		// bits 14-15, special move flag (Move_type::), promotion (1), en passant (2), castle (3)
		std::uint16_t move_data_{};
	};
}

#endif // Move_h_INCLUDED
