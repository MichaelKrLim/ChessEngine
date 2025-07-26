#include <doctest/doctest.h>

#include "Move.h"
#include "State.h"

using namespace engine;

TEST_SUITE("State.h")
{
	TEST_CASE("Constructor")
	{
		State state{"r4rk1/pppqbp1p/2n1pnp1/1b1p4/3PP3/4BNP1/PPPQNPBP/R4RK1 w - - 0 1"};
		REQUIRE(state.occupied_squares()==Bitboard{0x61bf740a1870ff61});
		for(const Castling_rights_map<bool> no_castling_rights{false, false}; const auto& side : all_sides)
		{
			const auto& current_side = state.sides[side];
			REQUIRE(current_side.castling_rights==no_castling_rights);
			Piece_map<int> expected_number{1, 8, 2, 2, 2, 1};
			for(const auto& piece : all_pieces)
				REQUIRE(current_side.pieces[piece].popcount()==expected_number[piece]);
		}
		REQUIRE(state.full_move_clock==1);
		REQUIRE(state.half_move_clock==0);
	}

	TEST_CASE("move()")
	{
		State state{"r4rk1/pppqbp1p/2n1pnp1/1b1p4/3PP3/4BNP1/PPPQNPBP/R4RK1 w - - 0 1"};
		const State state_copy{state};
		const Position origin_square{3, 4}, destination_square{4, 4};
		state.make(Move{origin_square, destination_square});

		CHECK_FALSE(state.piece_at(origin_square, Side::white));
		CHECK(state.piece_at(destination_square, Side::white)==Piece::pawn);
		CHECK(state.half_move_clock==0);
		CHECK(state.full_move_clock==1);
		CHECK(state.zobrist_hash!=state_copy.zobrist_hash);
	}

	TEST_CASE("unmove()")
	{
		struct Test_data
		{
			std::string_view fen{};
			Move move{};
		};

		std::array<Test_data, 3> fen_to_check {
			Test_data{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",             Move{Position{0, 1}, Position{2, 2}}}, // b1 -> c3
			Test_data{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", Move{Position{2, 5}, Position{3, 6}}}, // f3 -> g4
			Test_data{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                            Move{Position{4, 0}, Position{5, 0}}} // a5 -> a6
		};

		for(const auto& [fen, move] : fen_to_check)
		{
			State state{fen};
			const State state_copy{state};
			state.make(move);
			state.unmove();
			CHECK(state==state_copy);
		}
	}
}
