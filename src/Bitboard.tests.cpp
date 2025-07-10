#include <doctest/doctest.h>

#include "Bitboard.h"

#include <sstream>
#include <vector>

using namespace engine;

TEST_SUITE("Bitboard")
{
    TEST_CASE("Construction and conversion")
    {
        constexpr Bitboard b1{0xFF00};
        static_assert(static_cast<size_t>(b1) == 0xFF00);

        Bitboard b2;
        CHECK(static_cast<size_t>(b2) == 0);
    }

    TEST_CASE("Logical and bit operators")
    {
        constexpr Bitboard b1{0x0F0F};
        constexpr Bitboard b2{0x00FF};

        static_assert((b1 & b2) == (0x0F0F & 0x00FF));
        static_assert((b1 | b2) == (0x0F0F | 0x00FF));
        static_assert((~b1) == (~0x0F0F));
        static_assert((b1 << 4) == (0x0F0F << 4));
        static_assert((b1 >> 4) == (0x0F0F >> 4));
        static_assert((b1 * 2) == (0x0F0F * 2));
    }

    TEST_CASE("Assignment and compound operators")
    {
        Bitboard b{0x0F0F};

        b |= 0xF000;
        CHECK(static_cast<size_t>(b) == (0x0F0F | 0xF000));

        b &= 0xFFFF;
        CHECK(static_cast<size_t>(b) == ((0x0F0F | 0xF000) & 0xFFFF));

        b ^= 0x00FF;
        CHECK(static_cast<size_t>(b) == (((0x0F0F | 0xF000) & 0xFFFF) ^ 0x00FF));

        b >>= 4;
        CHECK(static_cast<size_t>(b) == ((((0x0F0F | 0xF000) & 0xFFFF) ^ 0x00FF) >> 4));

        Bitboard b2{0xF0F0};
        b |= b2;
        CHECK((static_cast<size_t>(b) & 0xF0F0) == 0xF0F0);

        b &= b2;
        CHECK(static_cast<size_t>(b) == (static_cast<size_t>(b) & 0xF0F0));
    }

    TEST_CASE("Comparison operators")
    {
        constexpr Bitboard b{0x1234};
        static_assert(b > 0x1000);
        static_assert(b < 0xFFFF);
        static_assert(b == 0x1234);
        static_assert(!(b == 0x5678));

        Bitboard b2{0x5678};
        CHECK(b != b2);
        CHECK(b != 0x5678);
        CHECK(!Bitboard{0});
    }

    TEST_CASE("Bitboard modification: add_piece, remove_piece, move_piece")
    {
        Bitboard b;
        const auto pos = Position{1, 2}; // c2

        b.add_piece(pos);
        CHECK(b.is_occupied(pos));

        b.remove_piece(pos);
        CHECK(!b.is_occupied(pos));

        const auto from = Position{0, 0}; // a1
        const auto to   = Position{0, 1}; // b1

        b.add_piece(from);
        CHECK(b.is_occupied(from));

        b.move_piece(from, to);
        CHECK(!b.is_occupied(from));
        CHECK(b.is_occupied(to));
    }

    TEST_CASE("popcount and lsb_square")
    {
        Bitboard b;
        b.add_piece(Position{0, 0}); // a1
        b.add_piece(Position{1, 0}); // a2
        CHECK(b.popcount() == 2);

        auto lsb = b.lsb_square();
        CHECK(lsb.rank_ == 0);
        CHECK(lsb.file_ == 0);
    }

    TEST_CASE("pretty_string output")
    {
        Bitboard b;
        b.add_piece(Position{7, 7}); // h8
        std::ostringstream oss;
        oss << b;
        auto str = oss.str();
        CHECK(str.contains('X'));         // C++23: string.contains
        CHECK(str.contains('h'));
    }

    TEST_CASE("for_each_piece")
    {
        Bitboard b;
        b.add_piece(Position{0, 0});
        b.add_piece(Position{1, 1});

        std::vector<Position> positions;
        b.for_each_piece([&](const auto& p) {
            positions.push_back(p);
        });
        CHECK(positions.size() == 2);
    }

    TEST_CASE("Free functions: is_free and rank_bb")
    {
        Bitboard b;
        b.add_piece(Position{0, 0}); // a1
        CHECK(!is_free(Position{0, 0}, b));
        CHECK(is_free(Position{0, 1}, b));
		constexpr auto rank0 = rank_bb(0);
		CHECK(rank0.popcount() == 8);

        static_assert(rank0.is_occupied(Position{0, 0}));
        static_assert(rank0.is_occupied(Position{0, 7}));
    }
}
