#include <doctest/doctest.h>

#include "State.h"
#include "Transposition_table.h"

using namespace engine;

void require_states_are_equal(const State& s1, const State& s2)
{
	REQUIRE(s1.zobrist_hash == s2.zobrist_hash);
	REQUIRE(s1.evaluation == s2.evaluation);
	REQUIRE(s1.side_to_move == s2.side_to_move);
	REQUIRE(s1.half_move_clock == s2.half_move_clock);
	REQUIRE(s1.full_move_clock == s2.full_move_clock);
	REQUIRE(s1.en_passant_target_square == s2.en_passant_target_square);
	for(const auto i : all_sides)
	{
		REQUIRE(s1.sides[i].castling_rights[Castling_rights::kingside] == s2.sides[i].castling_rights[Castling_rights::kingside]);
		REQUIRE(s1.sides[i].castling_rights[Castling_rights::queenside] == s2.sides[i].castling_rights[Castling_rights::queenside]);
	}
	for(const auto side_idx : all_sides)
		for(const auto piece_idx : all_pieces)
			REQUIRE(s1.sides[side_idx].pieces[piece_idx] == s2.sides[side_idx].pieces[piece_idx]);
}

TEST_SUITE("State")
{
	TEST_CASE("Make and Unmove Reversibility")
	{
		State state("rnbq1kr1/pp1Pbppp/2p5/8/2B4P/8/PPP1NnP1/RNBQK2R w KQ - 1 8");
		const State initial_state = state;
		std::stack<std::uint64_t> zobrist_history;
		std::stack<int> evaluation_history;
		std::vector moves = {
			Move(Position{3, 7}, Position{4, 7}), // 1. h4h5
			Move(Position{6, 7}, Position{5, 7}), // 1. h7h6
		};
		for(const auto& move : moves)
		{
			zobrist_history.push(state.zobrist_hash);
			evaluation_history.push(state.evaluation);
			state.make(move);
		}

		for(int i = moves.size() - 1; i >= 0; --i)
		{
			state.unmove();
			CHECK(state.zobrist_hash == zobrist_history.top());
			CHECK(state.evaluation == evaluation_history.top());
			zobrist_history.pop();
			evaluation_history.pop();
		}
		require_states_are_equal(state, initial_state);
	}

	TEST_CASE("En Passant Make/Unmove")
	{
		State state("rnbqkbnr/ppp2ppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
		const State initial_state = state;
		Move en_passant_move(Position{4, 4}, Position{5, 3}); // e5xd6
		state.make(en_passant_move);
		REQUIRE_FALSE(state.piece_at(Position{3, 3}, Side::black).has_value()); // Black pawn at d5 should be gone
		REQUIRE(state.piece_at(Position{5, 3}, Side::white) == Piece::pawn);   // White pawn should be at d6
		REQUIRE(state.side_to_move == Side::black);
		state.unmove();
		require_states_are_equal(state, initial_state);
	}
	
	TEST_CASE("Castling Make/Unmove")
	{
		State state("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
		const State initial_state_w = state;

		SUBCASE("White Kingside Castle")
		{
			Move castle_move(Position{0, 4}, Position{0, 6}); // O-O
			state.make(castle_move);
			REQUIRE(state.piece_at(Position{0, 6}, Side::white) == Piece::king);
			REQUIRE(state.piece_at(Position{0, 5}, Side::white) == Piece::rook);
			REQUIRE_FALSE(state.sides[Side::white].castling_rights[Castling_rights::kingside]);
			REQUIRE_FALSE(state.sides[Side::white].castling_rights[Castling_rights::queenside]);
			state.unmove();
			require_states_are_equal(state, initial_state_w);
		}
		
		SUBCASE("Black Queenside Castle")
		{
			state.side_to_move = Side::black;
			zobrist::invert_side_to_move(state.zobrist_hash);
			const State initial_state_b = state;
			Move castle_move(Position{7, 4}, Position{7, 2}); // o-o-o
			state.make(castle_move);
			REQUIRE(state.piece_at(Position{7, 2}, Side::black) == Piece::king);
			REQUIRE(state.piece_at(Position{7, 3}, Side::black) == Piece::rook);
			REQUIRE_FALSE(state.sides[Side::black].castling_rights[Castling_rights::kingside]);
			REQUIRE_FALSE(state.sides[Side::black].castling_rights[Castling_rights::queenside]);
			state.unmove();
			require_states_are_equal(state, initial_state_b);
		}
	}

	TEST_CASE("Promotion Make/Unmove")
	{
		State state("rnbq1bnr/pPppkppp/8/8/8/8/1PP1PPPP/RNBQKBNR w KQ - 1 5");
		const State initial_state = state;
		SUBCASE("Promotion to Queen without capture")
		{
			Move promotion_move(Position{6, 1}, Position{7, 1}, Piece::queen); // b7-b8=Q
			state.make(promotion_move);
			REQUIRE(state.piece_at(Position{7, 1}, Side::white) == Piece::queen);
			REQUIRE_FALSE(state.piece_at(Position{6, 1}, Side::white).has_value());
			state.unmove();
			require_states_are_equal(state, initial_state);
		}
		
		SUBCASE("Promotion to Knight with capture")
		{
			Move promotion_move(Position{6, 1}, Position{7, 0}, Piece::knight); // b7xa8=N
			state.make(promotion_move);
			REQUIRE(state.piece_at(Position{7, 0}, Side::white) == Piece::knight);
			REQUIRE_FALSE(state.piece_at(Position{6, 1}, Side::white).has_value());
			state.unmove();
			require_states_are_equal(state, initial_state);
		}
	}
}
