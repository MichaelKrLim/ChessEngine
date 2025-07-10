#include <doctest/doctest.h>

#include "Bitboard.h"
#include "Constants.h"
#include "Magic_generation_util.h"
#include "Magic_util.h"
#include "Pieces.h"
#include "Position.h"

using namespace engine;

TEST_SUITE("Magic utilities")
{
    TEST_CASE("rook_mask produces correct blockers")
	{
        Position d4{3,3}; // standard central square
        auto mask = magic::rook_mask(d4);
        // Should not include edges (rank/file=0,7)
        Bitboard bb{mask};
        CHECK_FALSE(bb.is_occupied(Position{3,0})); // d1
        CHECK_FALSE(bb.is_occupied(Position{3,7})); // d8
        CHECK(bb.is_occupied(Position{3,2})); // c4
        CHECK(bb.is_occupied(Position{4,3})); // d5
    }

    TEST_CASE("bishop_mask produces correct blockers")
	{
        Position d4{3,3};
        auto mask = magic::bishop_mask(d4);
        Bitboard bb{mask};
        // Should not include diagonals reaching board edge
        CHECK_FALSE(bb.is_occupied(Position{0,0})); // a1
        CHECK_FALSE(bb.is_occupied(Position{7,7})); // h8
        CHECK(bb.is_occupied(Position{2,2})); // c3
        CHECK(bb.is_occupied(Position{4,4})); // e5
    }

    TEST_CASE("generate_blocker_configurations size and contents")
	{
        Position e4{3,4};
        auto rook_blocks = magic::generate_blocker_configurations(e4, Piece::rook);
        auto bishop_blocks = magic::generate_blocker_configurations(e4, Piece::bishop);

        // Number of configurations: 2^(relevant_bits)
        auto rook_bits = magic::rook_relevant_bits[to_index(e4)];
        auto bishop_bits = magic::bishop_relevant_bits[to_index(e4)];
        CHECK(rook_blocks.size() == (1ULL << rook_bits));
        CHECK(bishop_blocks.size() == (1ULL << bishop_bits));

        // First config must be empty
        CHECK(static_cast<std::uint64_t>(rook_blocks.front()) == 0ULL);
        CHECK(static_cast<std::uint64_t>(bishop_blocks.front()) == 0ULL);
    }

    TEST_CASE("rook_reachable_squares finds moves until blockers")
	{
        Position d4{3,3};
        Bitboard occupied{};
        // Block square in front: d5
        occupied.add_piece(Position{4,3});
        auto moves = magic::rook_reachable_squares(d4, occupied);
        CHECK(moves.is_occupied(Position{4,3})); // d5 should be included (capture)
        CHECK_FALSE(moves.is_occupied(Position{5,3})); // d6 should be blocked
    }

    TEST_CASE("bishop_reachable_squares finds moves until blockers")
	{
        Position d4{3,3};
        Bitboard occupied{};
        // Block e5
        occupied.add_piece(Position{4,4});
        auto moves = magic::bishop_reachable_squares(d4, occupied);
        CHECK(moves.is_occupied(Position{4,4})); // e5 included
        CHECK_FALSE(moves.is_occupied(Position{5,5})); // f6 blocked
    }

    TEST_CASE("find_magic generates valid magic square")
	{
        Position d4{3,3};

        auto magic_rook = magic::find_magic(d4, Piece::rook);
        auto magic_bishop = magic::find_magic(d4, Piece::bishop);

        // Shift must match relevant bits
        CHECK(magic_rook.shift == (64 - magic::rook_relevant_bits[to_index(d4)]));
        CHECK(magic_bishop.shift == (64 - magic::bishop_relevant_bits[to_index(d4)]));

        // Mask must equal rook_mask/bishop_mask
        CHECK(magic_rook.mask == magic::rook_mask(d4));
        CHECK(magic_bishop.mask == magic::bishop_mask(d4));

        // Attack table size: 2^relevant_bits
        CHECK(magic_rook.attack_table.size() == (1ULL << magic::rook_relevant_bits[to_index(d4)]));
        CHECK(magic_bishop.attack_table.size() == (1ULL << magic::bishop_relevant_bits[to_index(d4)]));
    }
}