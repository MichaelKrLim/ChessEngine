#ifndef Bitboards_h_INCLUDED
#define Bitboards_h_INCLUDED

#include "Bitboard.h"
#include "Board.h"
#include "Constants.h"
#include "Pieces.h"

#include <array>
#include <unordered_map>
#include <vector>


namespace std 
{
	template <>
	struct hash<engine::Position>
	{
		size_t operator()(const engine::Position& p) const
		{
			size_t h1 = std::hash<int>{}(p.rank_);
			size_t h2 = std::hash<int>{}(p.file_);
			return h1 ^ (h2 << 1);
		}
	};
}

namespace engine
{	
	class Move_generator
	{
		public:

		Move_generator();
		using moves_type = std::unordered_map<Position, std::vector<Position>>;
		[[nodiscard]] static const std::array<moves_type, number_of_piece_types> get(const Board& board);
		void perft(const int depth, const std::string fen, const std::string continuation) const;

		private:

		struct Magic_square
		{
			const std::vector<Bitboard>* attack_table;
			std::uint64_t mask;
			std::uint64_t magic;
			std::uint8_t shift;
		};

		static std::vector<std::vector<Bitboard>> create_attack_table();
		static std::vector<Bitboard> blocker_configurations(const Position& square, const bool& bishop);
		void cast_magic();

		[[nodiscard]] static const moves_type pawn_legal_moves(const Bitboard& pawn_bb, const Bitboard& occupied_squares, const Side& active_player);
		[[nodiscard]] static const moves_type king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares);
		[[nodiscard]] static const moves_type knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares);
		[[nodiscard]] static const moves_type bishop_legal_moves(const Bitboard& bishop_bb, const Bitboard& occupied_squares);
		[[nodiscard]] static const moves_type rook_legal_moves(const Bitboard& rook_bb, const Bitboard& occupied_squares);
		[[nodiscard]] static const moves_type queen_legal_moves(const Bitboard& queen_bb, const Bitboard& occupied_squares);

		[[nodiscard]] static const Bitboard bishop_legal_moves_bb(const Position& original_square, const Bitboard& occupied_squares);
		[[nodiscard]] static const Bitboard rook_legal_moves_bb(const Position& original_square, const Bitboard& occupied_squares);
		[[nodiscard]] static const Bitboard rook_reachable_squares(const Position& square, const Bitboard& occupied_squares);
		[[nodiscard]] static const Bitboard bishop_reachable_squares(const Position& square, const Bitboard& occupied_squares);

		[[nodiscard]] const std::uint64_t random_uint64() const;
		[[nodiscard]] const std::uint64_t random_uint64_fewbits() const;
		[[nodiscard]] const Magic_square find_magic(const Position& square, std::uint64_t mask, const std::vector<Bitboard>& blocker_configurations, const std::uint8_t rellevant_bits, const std::vector<Bitboard>& attack_table) const;
		[[nodiscard]] const std::uint64_t rook_mask(const Position& square) const;
		[[nodiscard]] const std::uint64_t bishop_mask(const Position& square) const;
		[[nodiscard]] inline const std::size_t magic_hash(const Bitboard& key, const std::uint64_t& magic, int rellevant_bits) const { return (key*magic) >> (64-rellevant_bits); }

		static std::array<Magic_square, 64> bishop_magic_squares_;
		static std::array<Magic_square, 64> rook_magic_squares_;
		static std::vector<std::vector<Bitboard>> attack_table_;

		constexpr static std::array<std::uint8_t, board_size*board_size> rook_rellevant_bits = 
		{
			12, 11, 11, 11, 11, 11, 11, 12,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			12, 11, 11, 11, 11, 11, 11, 12
		};

		constexpr static std::array<std::uint8_t, board_size*board_size> bishop_rellevant_bits = 
		{
			6, 5, 5, 5, 5, 5, 5, 6,
			5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 7, 7, 7, 7, 5, 5,
			5, 5, 7, 9, 9, 7, 5, 5,
			5, 5, 7, 9, 9, 7, 5, 5,
			5, 5, 7, 7, 7, 7, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5,
			6, 5, 5, 5, 5, 5, 5, 6
		};

		constexpr static std::array<Position, 8> knight_moves_ =
		{{
			Position{2, 1}, Position{2, -1}, Position{-2, 1}, Position{-2, -1},
			Position{1, 2}, Position{1, -2}, Position{-1, 2}, Position{-1, -2}
		}};
		constexpr static std::array<Position, 4> bishop_moves_ =
		{{
			Position{1,  1}, Position{1,  -1},
			Position{-1, 1}, Position{-1, -1}
		}};
		constexpr static std::array<Position, 8> king_moves_ =
		{{
			Position{1, 0}, Position{-1, 0}, Position{0,  1}, Position{0,  -1},
			Position{1, 1}, Position{-1, 1}, Position{1, -1}, Position{-1, -1}
		}};
	};
}

#endif // Bitboards_h_INCLUDED
