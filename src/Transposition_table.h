#ifndef Transposition_table_h_INCLUDED
#define Transposition_table_h_INCLUDED

#include "Constants.h"
#include "Move.h"
#include "Pieces.h"
#include "State.h"

#include <array>
#include <cstdint>
#include <mutex>
#include <optional>
#include <random>
#include <shared_mutex>
#include <vector>

namespace zobrist
{
	const static struct Zobrist_randoms
	{
		engine::Piece_map<engine::Side_map<std::array<std::uint64_t, engine::board_size*engine::board_size>>> pieces;
		engine::Side_map<engine::Castling_rights_map<std::uint64_t>> castling_rights;
		std::array<std::uint64_t, engine::board_size*engine::board_size> en_passant_squares;
		std::uint64_t side;
	} zobrist_randoms = []()
	{
		Zobrist_randoms zobrist_randoms;
		std::mt19937_64 rng(49293);
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

		zobrist_randoms.side = dist(rng);
		return zobrist_randoms;
	}();

	inline void invert_piece_at(std::uint64_t& hash, const engine::Position& position, const engine::Piece& piece, const engine::Side& side)
	{
		hash ^= zobrist_randoms.pieces[piece][side][to_index(position)];
	}

	inline void invert_castling_right(std::uint64_t& hash, const engine::Side& side, const engine::Castling_rights& castling_right)
	{
		hash ^= zobrist_randoms.castling_rights[side][castling_right];
	}

	inline void invert_en_passant_square(std::uint64_t& hash, const engine::Position& position)
	{
		hash ^= zobrist_randoms.en_passant_squares[to_index(position)];
	}

	inline void invert_side_to_move(std::uint64_t& hash)
	{
		hash ^= zobrist_randoms.side;
	}

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
		if(state.side_to_move == engine::Side::white)
			hash ^= zobrist_randoms.side;
		return hash;
	}
}

namespace engine
{
	enum class Search_result_type
	{
		lower_bound, upper_bound, exact, size
	};

	struct Transposition_data
	{
		unsigned remaining_depth;
		int eval;
		std::uint64_t zobrist_hash;
		Search_result_type search_result_type;
		engine::Move best_move;
	};

	class Transposition_table
	{
		public:

		[[nodiscard]] std::optional<Transposition_data> operator[](const std::uint64_t& zobrist_hash)
		{
			const auto index{zobrist_hash%data.size()};
			std::shared_lock resizing_lock(tt_mtx);
			std::shared_lock group_lock(locks[index%num_lock_groups]);

			const Transposition_data entry{data[index]};
			if(entry.zobrist_hash==zobrist_hash)
				return entry;
			else
				return std::nullopt;
		}

		void insert(const Transposition_data& t_data)
		{
			const auto index = t_data.zobrist_hash%data.size();
			std::shared_lock resizing_lock(tt_mtx);
			std::lock_guard group_lock(locks[index%num_lock_groups]);

			data[index] = t_data;
		}

		void resize(int size_mb) noexcept
		{
			std::lock_guard lock(tt_mtx);

			data.resize((size_mb*mb_to_bytes)/sizeof(Transposition_data));
			num_lock_groups=compute_num_lock_groups();
			std::vector<std::shared_mutex> new_locks(num_lock_groups);
			locks.swap(new_locks);
		}

		void clear() noexcept
		{
			std::lock_guard lock(tt_mtx);

			for(auto& entry : data)
				entry.zobrist_hash=0;
		}

		explicit Transposition_table(int table_size_mb)
			: data((table_size_mb*mb_to_bytes)/sizeof(Transposition_data))
			, num_lock_groups(compute_num_lock_groups())
			, locks(num_lock_groups)
		{};

		private:

		[[nodiscard]] inline int compute_num_lock_groups()
		{
			return std::clamp(static_cast<int>(data.size())/1000, 10, 10000);
		}

		std::vector<Transposition_data> data;
		int num_lock_groups{-1};
		std::vector<std::shared_mutex> locks;
		std::shared_mutex tt_mtx;

		constexpr static int mb_to_bytes{1024*1024};
	};
} // namespace engine

#endif
