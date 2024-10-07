#ifndef Board_state_h_INCLUDED
#define Board_state_h_INCLUDED

#include "Board.h"
#include "Pieces.h"
#include "Position.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace engine
{
	class Board_state
	{
		public:

		inline explicit Board_state(const Board& board) : board_(board) {}
		inline Board_state() : board_(Board(FEN())) {};

		[[nodiscard]] bool white_is_mated() const;
		[[nodiscard]] bool black_is_mated() const;
		[[nodiscard]] bool is_draw() const;
		void output_board() const;
		
		private:

		Board board_;

		void init_black_weightmaps();
	};
}

#endif // Board_state_h_INCLUDED
