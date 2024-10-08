#ifndef Move_h_INCLUDED
#define Move_h_INCLUDED

#include "Constants.h"
#include "Pieces.h"
#include "Square.h"

#include <cassert>
#include <cstdint>

enum class Move_type
{
	normal,
	promotion = 1<<14,
	en_passant = 2<<14,
	castling = 3<<14
};

namespace engine
{
	class Move
	{
		public:

		Move() = default;
		constexpr explicit Move(std::uint16_t move_data) : move_data_(move_data) {}
		constexpr Move(Square current_position, Square desired_position) :
			move_data_((current_position<<6) + desired_position) {}

		template<Move_type move_type>
		static constexpr Move make(Square current_position, Square desired_position, Piece piece_type = Piece::knight)
		{
			return Move(static_cast<std::uint16_t>(move_type)+((static_cast<std::uint16_t>(piece_type)-static_cast<std::uint16_t>(Piece::knight))<<12)+(current_position<<6)+desired_position);
		}
		constexpr Square from_square() const
		{
			assert(move_data_ != null().move_data_ && move_data_ != none().move_data_);
			return Square((move_data_>>6)&board_size*board_size-1);
		}
		constexpr Square destination_square() const
		{
			assert(move_data_ != null().move_data_ && move_data_ != none().move_data_);
			return Square(move_data_&board_size*board_size-1);
		}

		static constexpr Move null() { return Move(0b1000001); }
		static constexpr Move none() { return Move(0); }
		
		private:

		std::uint16_t move_data_;
	};
}

#endif // Move_h_INCLUDED
