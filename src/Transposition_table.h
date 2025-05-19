#ifndef Transposition_table_h_INCLUDED
#define Transposition_table_h_INCLUDED

#include "Pieces.h"
#include "State.h"

#include <array>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace zobrist
{
	const static struct Zobrist_randoms
	{
		engine::Piece_map<engine::Side_map<std::array<std::uint64_t, 64>>> pieces;
		engine::Side_map<engine::Castling_rights_map<std::uint64_t>> castling_rights;
		std::array<std::uint64_t, engine::to_index(engine::Position{7, 7})> en_passant_squares;
	} zobrist_randoms = []()
	{
		Zobrist_randoms zobrist_randoms;
		std::mt19937_64 rng(std::random_device{}());
		std::uniform_int_distribution<uint64_t> dist;
		for (auto &side_container : zobrist_randoms.pieces)
			for (auto &position_container : side_container)
				for (auto &position : position_container)
					position = dist(rng);

		for(auto& random : zobrist_randoms.en_passant_squares)
			random = dist(rng);

		for(auto& side_randoms : zobrist_randoms.castling_rights)
			for(auto& random : side_randoms)
				random = dist(rng);
		return zobrist_randoms;
	}();

	[[nodiscard]] inline std::uint64_t hash(const engine::State& state) noexcept
	{
		std::uint64_t hash{0};
		for(const auto &[piece, position, side] : state.get_board_data())
		{
			hash ^= zobrist_randoms.pieces[piece][side][to_index(position)];
		}
		if(state.en_passant_target_square)
			hash ^= zobrist_randoms.en_passant_squares[to_index(state.en_passant_target_square.value())];
		for(const auto& side : engine::all_sides)
		{
			for(const auto& castling_right : engine::all_castling_rights)
			{
				if(state.sides[side].castling_rights[castling_right])
				{
					hash ^= zobrist_randoms.castling_rights[side][castling_right];
				}
			}
		}
		return hash;
	}
}

namespace engine
{
	struct Transposition_data
	{
		unsigned depth;
		double eval;
		engine::Side to_move;
		std::uint64_t zobrist_hash;
	};

	class Transposition_table
	{
		public:

		[[nodiscard]] std::optional<Transposition_data>& operator[](const State& state)
		{
			const auto index = zobrist::hash(state) % data.size();
			return data[index];
		}

		explicit Transposition_table(const unsigned table_size_log2) : data(1ULL<<table_size_log2, std::nullopt) {};

		private:

		std::vector<std::optional<Transposition_data>> data;
	};
}

#endif
