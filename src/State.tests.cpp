#include <doctest/doctest.h>

#include "State.h"
#include "Move.h"
#include "Move_generator.h"
#include "Chess_data.h"
#include "Transposition_table.h"

using namespace engine;

TEST_SUITE("State")
{
    TEST_CASE("State constructor from valid FEN initializes correctly")
    {
        State state{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

        CHECK(state.side_to_move == Side::white);
        CHECK(state.full_move_clock == 1);
        CHECK(state.half_move_clock == 0);
        CHECK(!state.get_board_data().empty());
    }

    TEST_CASE("piece_at returns correct piece")
    {
        State state{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"};
        auto piece = state.piece_at(Position{0, 0}, Side::white);
        CHECK(piece.has_value());
        CHECK(piece.value() == Piece::rook);
    }

	TEST_CASE("make and unmove from complex position: e2e4")
	{
		// Original FEN
		State state{"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"};
		// Save initial state
		auto before_hash = state.zobrist_hash;
		auto before_eval = state.evaluation;
		auto before_side = state.side_to_move;
		auto before_halfmove = state.half_move_clock;
		auto before_fullmove = state.full_move_clock;
		auto before_en_passant = state.en_passant_target_square;
		auto before_board = state.get_board_data();
		auto before_occupied = state.occupied_squares();

		// Move pawn e2 -> e4 (rank 1, file 4) -> (rank 3, file 4)
		Move move{Position{1, 4}, Position{3, 4}};
		state.make(move);

		// Check side to move flipped to black
		CHECK(state.side_to_move == Side::black);

		// Halfmove clock resets (pawn move)
		CHECK(state.half_move_clock == 0);

		// Full move clock stays at 8 (since white just moved)
		CHECK(state.full_move_clock == before_fullmove);

		// En passant target should now be e3 (rank 2, file 4)
		REQUIRE(state.en_passant_target_square.has_value());
		CHECK(state.en_passant_target_square.value() == Position{2, 4});

		// History stack should now have 1 entry
		CHECK(state.history.size() == 1);

		// Zobrist hash and evaluation must change
		CHECK(state.zobrist_hash != before_hash);
		CHECK(state.evaluation != before_eval);

		// Occupied squares must be different
		CHECK(state.occupied_squares() != before_occupied);

		// The pawn should now be on e4
		auto piece_e4 = state.piece_at(Position{3, 4}, Side::white);
		REQUIRE(piece_e4.has_value());
		CHECK(piece_e4.value() == Piece::pawn);

		// e2 must now be empty
		auto piece_e2 = state.piece_at(Position{1, 4}, Side::white);
		CHECK_FALSE(piece_e2.has_value());

		// --- Undo the move ---
		state.unmove();

		// Should return to original state
		CHECK(state.side_to_move == before_side);
		CHECK(state.half_move_clock == before_halfmove);
		CHECK(state.full_move_clock == before_fullmove);
		CHECK(state.zobrist_hash == before_hash);
		CHECK(state.evaluation == before_eval);
		CHECK(state.history.empty());
		CHECK(state.en_passant_target_square == before_en_passant);

		// Pawn back on e2
		auto piece_e2_after = state.piece_at(Position{1, 4}, Side::white);
		REQUIRE(piece_e2_after.has_value());
		CHECK(piece_e2_after.value() == Piece::pawn);
	}


    TEST_CASE("occupied_squares returns non-zero bitboard")
    {
        State state{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"};
        auto occupied = state.occupied_squares();
        CHECK(occupied != 0);
    }

    TEST_CASE("in_check returns false for starting position")
    {
        State state{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"};
        CHECK_FALSE(state.in_check());
    }

    TEST_CASE("get_board_data returns correct number of pieces at start")
    {
        State state{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"};
        auto data = state.get_board_data();
        CHECK(data.size() == 32); // 16 pieces per side
    }

}