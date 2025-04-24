#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Board.h"
#include "Move.h"

namespace engine
{
	class Engine
	{
		public:

		[[nodiscard]] Move generate_move_at_depth(Board board, const int depth) noexcept;
		inline void set_position(const std::string_view& fen) { board = Board{fen}; }

		private:

		using weightmap_type = Piece_map<std::array<int, board_size*board_size>>;
		static constexpr Side_map<weightmap_type> weightmaps;
		static constexpr Piece_map<int> piece_values;

		[[nodiscard]] double evaluate(const Board& board) noexcept;

		Board board;

		friend Uci_handler;
	};
}

#endif //Engine_h_INCLUDED
