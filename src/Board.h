#ifndef Board_h_INCLUDED
#define Board_h_INCLUDED

#include "Bitboard.h"
#include "Constants.h"
#include "Enum_map.h"
#include "Move.h"
#include "Position.h"

#include <array>
#include <cctype>
#include <optional>
#include <stack>

namespace engine
{
	enum class Castling_rights
	{
		kingside, queenside, size
	};
	template <typename Mapped_type>
	using Castling_rights_map = Enum_map_from_size<Castling_rights, Mapped_type>;

	struct Side_position
	{
		Enum_map<Piece, Bitboard, number_of_pieces> pieces{};
		Castling_rights_map<bool> castling_rights{true, true};

		[[nodiscard]] inline Bitboard occupied_squares() const noexcept 
		{
			Bitboard occupied_squares{0ULL};
			for(const auto& piece_bb : pieces)
				occupied_squares |= piece_bb;
			return occupied_squares;
		};
	};

	struct Board
	{
		private:

		struct State_delta
		{
			const Move move;
			const Piece piece;
			const std::optional<Piece> captured_piece;
			const Bitboard enemy_attack_map;
			const std::optional<Position> en_passant_target_square;
			const bool was_en_passant;
			Castling_rights_map<bool> castling_rights;
		};

		void validate_fen(const std::array<std::string, 6>& partitioned_fen) const;
		void parse_fen(const std::string_view fen) noexcept;
		[[nodiscard]] std::optional<Piece> piece_at(const Position& position, const Side& side) const noexcept;
		void update_castling_rights(const Side& side) noexcept;

		public:

		explicit Board(const std::string_view fen);
		Board() = default;

		Enum_map<Side, Side_position, 2> sides{};
		int half_move_clock{}, full_move_clock{};
		Side side_to_move{Side::white};
		Bitboard enemy_attack_map;
		std::optional<Position> en_passant_target_square{std::nullopt};
		static std::stack<State_delta> history;

		void make(const Move& move) noexcept;
		void unmove() noexcept;
		[[nodiscard]] Bitboard occupied_squares() const noexcept;
		[[nodiscard]] inline bool is_square_attacked(const Position& position) const noexcept { return enemy_attack_map.is_occupied(position); }
		[[nodiscard]] bool in_check() const noexcept;
	};
}

#endif // Board_h_INCLUDED
