#ifndef State_h_INCLUDED
#define State_h_INCLUDED

#include "Bitboard.h"
#include "Constants.h"
#include "Enum_map.h"
#include "Move.h"
#include "Position.h"

#include <algorithm>
#include <array>
#include <optional>
#include <stack>
#include <vector>

namespace engine
{
	enum class Castling_rights
	{
		kingside, queenside, size
	};
	template <typename Mapped_type>
	using Castling_rights_map = Enum_map_from_size<Castling_rights, Mapped_type>;
	constexpr auto all_castling_rights = {Castling_rights::kingside, Castling_rights::queenside};

	inline std::ostream& operator<<(std::ostream& os, const Castling_rights& castling_right)
	{
		return os << (castling_right==Castling_rights::kingside? "kingside" : "queenside");
	}

	struct Side_position
	{
		Enum_map<Piece, Bitboard, number_of_pieces> pieces{};
		Castling_rights_map<bool> castling_rights{false, false};

		[[nodiscard]] inline Bitboard occupied_squares() const noexcept 
		{
			Bitboard occupied_squares{0ULL};
			for(const auto& piece_bb : pieces)
				occupied_squares |= piece_bb;
			return occupied_squares;
		};
	};

	inline std::ostream& operator<<(std::ostream& os, const Castling_rights_map<bool>& castling_rights_map)
	{
		bool is_first{true};
		for(const auto& castling_right : all_castling_rights)
		{
			os << castling_right << ": " << castling_rights_map[castling_right];
			if(is_first)
				os << "\n";
			is_first = false;
		}
		return os;
	}

	struct State
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
			const Castling_rights_map<bool> white_castling_rights;
			const Castling_rights_map<bool> black_castling_rights;
			const std::uint64_t previous_zobrist_hash;
			const unsigned half_move_clock;
			const double evaluation;
		};

		struct Piece_and_data
		{
			Piece piece;
			Position position;
			Side side;
		};

		void validate_fen(const std::array<std::string, 6>& partitioned_fen) const;
		void parse_fen(const std::string_view fen) noexcept;
		void update_castling_rights(const Side& side) noexcept;
		void move_and_hash(const Position& from_square, const Position& destination_square, const Piece& piece_type_to_move) noexcept;

		public:

		explicit State(const std::string_view fen);
		State() = default;

		Enum_map<Side, Side_position, 2> sides{};
		unsigned half_move_clock{}, full_move_clock{};
		Side side_to_move{Side::white};
		Bitboard enemy_attack_map;
		std::optional<Position> en_passant_target_square{std::nullopt};
		std::uint64_t zobrist_hash;
		std::vector<std::uint64_t> repetition_history;
		std::stack<State_delta> history{};
		double evaluation;

		void make(const Move& move) noexcept;
		void unmove() noexcept;
		[[nodiscard]] Bitboard occupied_squares() const noexcept;
		[[nodiscard]] inline bool is_square_attacked(const Position& position) const noexcept { return enemy_attack_map.is_occupied(position); }
		[[nodiscard]] bool in_check() const noexcept;
		[[nodiscard]] std::vector<Piece_and_data> get_board_data() const noexcept;
		[[nodiscard]] bool is_stalemate() const noexcept;
		[[nodiscard]] std::optional<Piece> piece_at(const Position& position, const Side& side) const noexcept;

		friend std::ostream& operator<<(std::ostream& os, const State& state);
	};

	inline std::ostream& operator<<(std::ostream& os, const State& state)
	{
		os << "hash: " << state.zobrist_hash << "\n"
		<< "repetition count: " << std::ranges::count(state.repetition_history | std::views::reverse | std::views::take(50), state.zobrist_hash) << "\n"
		<< state.occupied_squares() << "\n"
		<< "half move clock: " << state.half_move_clock << "\n"
		<< "full move clock: " << state.full_move_clock << "\n"
		<< "side to move: " << (state.side_to_move==Side::white?"white":"black") << "\n";
		if(state.en_passant_target_square)
			os << "en_passant_target_square: " << state.en_passant_target_square.value() << "\n";
		for(const auto& side : all_sides)
		{
			const auto current_side = state.sides[side];
			os << side << ":\n" << current_side.castling_rights << "\n";
		}
		return os;
	}
}

#endif // State_h_INCLUDED
