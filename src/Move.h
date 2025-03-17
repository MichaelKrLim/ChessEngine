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

		constexpr inline Move() = default;
		constexpr inline Move(const Position& current_square, const Position& desired_square) :
			move_data_((to_index(current_square)<<6) | to_index(desired_square)) {}

		static inline constexpr Move make(Position current_square, Position desired_square, Move_type move_type, Piece piece_type = Piece::knight)
		{
			return Move(static_cast<std::uint16_t>(move_type)<<14|
					  ((static_cast<std::uint16_t>(piece_type)-static_cast<std::uint16_t>(Piece::knight))<<12)|
										 (to_index(current_square)<<6)|
										  to_index(desired_square));
		}

		inline constexpr Position from_square() const
		{
			return Position((move_data_>>6) & (board_size*board_size-1));
		}

		inline constexpr Position destination_square() const
		{
			return Position(move_data_ & (board_size*board_size-1));
		}

		inline constexpr Move_type type() const
		{
			return static_cast<Move_type>((move_data_ >> 13) & 0b11);
		}

		inline constexpr Piece promotion_piece() const
		{
			return static_cast<Piece>((move_data_ >> 11) & 0b11);
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
