#include <doctest/doctest.h>

#include "State.h"
#include "Transposition_table.h"

#include <unordered_set>

using namespace	engine;

TEST_SUITE("Transposition Table")
{
	TEST_CASE("Zobrist")
	{
		SUBCASE("generated randoms")
		{
			std::unordered_set<std::uint64_t> used_randoms{};
			for(auto &side_container : zobrist::zobrist_randoms.pieces)
				for(auto &position_container : side_container)
					for(auto &position : position_container)
					{
						CHECK_FALSE(used_randoms.contains(position));
						used_randoms.insert(position);
					}

			for(auto& random : zobrist::zobrist_randoms.en_passant_squares)
			{
				CHECK_FALSE(used_randoms.contains(random));
				used_randoms.insert(random);
			}

			for(auto& side_randoms : zobrist::zobrist_randoms.castling_rights)
				for(auto& random : side_randoms)
				{
					CHECK_FALSE(used_randoms.contains(random));
					used_randoms.insert(random);
				}

			CHECK_FALSE(used_randoms.contains(zobrist::zobrist_randoms.side));
		}
		
		SUBCASE("invert_piece_at")
		{
			std::uint64_t hash{594392934};
			const std::uint64_t	old_hash{hash};
			zobrist::invert_piece_at(hash, Position{1, 1}, Piece::rook, Side::black);
			zobrist::invert_piece_at(hash, Position{1, 1}, Piece::rook, Side::black);
			CHECK(hash==old_hash);
		}

		SUBCASE("invert_castling_right")
		{
			std::uint64_t hash{347789696567};
			const std::uint64_t	old_hash{hash};
			zobrist::invert_castling_right(hash, Side::black, Castling_rights::kingside);
			zobrist::invert_castling_right(hash, Side::black, Castling_rights::kingside);
			CHECK(hash==old_hash);
		}

		SUBCASE("invert_en_passant_square")
		{
			std::uint64_t hash{546544745754745};
			const std::uint64_t	old_hash{hash};
			zobrist::invert_en_passant_square(hash,	Position{2, 2});
			zobrist::invert_en_passant_square(hash,	Position{2, 2});
			CHECK(hash==old_hash);
		}

		SUBCASE("invert_side_to_move")
		{
			std::uint64_t hash{546544745754745};
			const std::uint64_t	old_hash{hash};
			zobrist::invert_side_to_move(hash);
			zobrist::invert_side_to_move(hash);
			CHECK(hash==old_hash);
		}

		SUBCASE("hash(State)")
		{
			State state{starting_fen};
			const auto hash{zobrist::hash(state)};
			REQUIRE(hash==zobrist::hash(state));

			state.make(Move{Position{1, 1}, Position{3, 1}});
			CHECK(zobrist::hash(state)!=hash);
		}
	}

	TEST_CASE("Transposition Table")
	{
		SUBCASE("Operator[] and Insertion")
		{
			Transposition_table tt(100);
			CHECK(tt[0x234234234]==std::nullopt);
			const auto some_hash{0x239482342342342};
			tt.insert(Transposition_data{
				.eval=1000,
				.zobrist_hash=some_hash,
			});
			const auto cache_result{tt[some_hash]};
			REQUIRE(cache_result.has_value());
			CHECK(cache_result->eval==1000.0);
			CHECK(tt[0x234234234]==std::nullopt);
		}
	}
}
