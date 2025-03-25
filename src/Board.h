#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Bitboard.h"
#include "Constants.h"
#include "Move.h"
#include "Position.h"

#include <array>
#include <cctype>
#include <optional>
#include <stack>

namespace engine
{
	struct Side_position
	{
		std::array<Bitboard, number_of_pieces> pieces{};
		std::optional<Position> en_passent_target_square{std::nullopt};
		std::array<bool, 2> castling_rights{false, false};

		[[nodiscard]] inline Bitboard occupied_squares() const noexcept 
		{
			Bitboard occupied_squares{0ULL};
			for(const auto& piece_bb : pieces)
				occupied_squares |= piece_bb;
			return occupied_squares;
		};
	};

	enum class Castling_rights
	{
		kingside, queenside
	};

	struct Board
	{
		private:

		struct State_delta
		{
			const Move move;
			const Piece piece;
			std::optional<Piece> captured_piece;
			const Bitboard enemy_attack_map;
		};

		public:

		explicit Board(const std::string_view& fen_string);
		Board() = default;

		std::array<Side_position, 2> sides{};
		int half_move_clock{}, full_move_clock{};
		Side side_to_move{Side::white};
		Bitboard enemy_attack_map;
		static std::stack<State_delta> history;

		void make(const Move& move);
		void unmove();
		[[nodiscard]] Bitboard occupied_squares() const noexcept;
		[[nodiscard]] inline bool is_square_attacked(const Position& position) const noexcept { return enemy_attack_map.is_occupied(position); }
		[[nodiscard]] bool in_check() const noexcept;
	};
}

#endif // Board_h_INCLUDED
