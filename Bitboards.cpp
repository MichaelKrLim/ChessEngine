#include "Bitboards.h"

#include "Constants.h"
#include "Position.h"

using namespace engine;

constexpr Bitboards::Bitboards()
{
	cast_magic();
}

const std::unordered_map<Piece, std::uint64_t> Bitboards::get(const Side_position &, const bool &is_white_to_move) const
{
    
}

// official-stockfish (2024). Stockfish/src/bitboard.cpp at 9766db8139ce8815110c15bdde8381d0564a63fa · official-stockfish/Stockfish. [online] GitHub. Available at: https://github.com/official-stockfish/Stockfish/blob/9766db8139ce8815110c15bdde8381d0564a63fa/src/bitboard.cpp [Accessed 9 Jan. 2025].
std::string Bitboards::output(Bitboard b) 
{

    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (std::size_t r = 7; r >= 0; --r)
    {
        for (std::size_t f = 0; f <= 7; ++f)
            s += b & (1ULL << f+r*board_size) ? "| X " : "|   ";

        s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
    }
    s += "  a   b   c   d   e   f   g   h\n";

    return s;
}


//raw cgpt to fix
const std::uint64_t Bitboards::pawn_legal_moves(const std::uint64_t& white_pawns, const std::uint64_t black_pawns,
	const std::uint64_t& occupied_squares, const bool& is_white_to_move
) const
{

}

const std::uint64_t Bitboards::knight_legal_moves(const Position& knight_square, const std::uint64_t& occupied_squares) const
{
	std::uint64_t valid_moves{0};
	for(const auto& move : knight_moves)
	{
		const auto [del_rank, del_file] = move;
		const Position destination_square(knight_square.rank_+del_rank, knight_square.file_+del_file);
		if(is_valid_destination(destination_square, occupied_squares))
			valid_moves |= 1 << to_index(destination_square);
	}
	return valid_moves;
}

const std::uint64_t Bitboards::king_legal_moves(const Position& king_square, const std::uint64_t& occupied_squares) const
{
	std::uint64_t valid_moves{0};
	for(const auto& move : king_moves)
	{
		const auto [del_rank, del_file] = move;
		const Position destination_square(king_square.rank_+del_rank, king_square.file_+del_file);
		if(is_valid_destination(destination_square, occupied_squares))
			valid_moves |= 1 << to_index(destination_square);
	}
	return valid_moves;
}

constexpr const std::uint64_t Bitboards::rook_reachable_squares(const Position& rook_square, const std::uint64_t& occupied_squares) const
{
	std::uint64_t valid_moves{};
	for(int del_rank{0}; del_rank < board_size; ++del_rank)
	{
		for(int del_file{0}; del_file < board_size; ++del_file)
		{
			const Position destination_square = rook_square + Position{del_rank, del_file};
			if(is_valid_destination(destination_square, occupied_squares))
				valid_moves |= (1 << to_index(destination_square));
			else 
				break;
		}
	}
	return valid_moves;
}

constexpr const std::uint64_t Bitboards::bishop_reachable_squares(const Position& bishop_square, const std::uint64_t& occupied_squares) const
{
	const auto explore_diagonal = [](this auto&& rec, const Position& bishop_square, const Position& diagonal_offset,
		const std::uint64_t occupied_squares, std::uint64_t& valid_moves,
		const Position& original_bishop_square)
	{
		const Position destination_square = bishop_square + diagonal_offset;
		if(!is_valid_destination(destination_square, occupied_squares))
			return;
		valid_moves |= (1 << to_index(destination_square));

		rec(destination_square, diagonal_offset, occupied_squares, valid_moves, original_bishop_square);
	};

	std::uint64_t valid_moves{};
	for(const auto& direction : bishop_moves)
	{
		explore_diagonal(bishop_square, direction, occupied_squares, valid_moves, bishop_square);
	}
	return valid_moves;
}

constexpr void Bitboards::initialise_attack_table()
{
	for(const auto& moves : bishop_moves())
	{

	}
}

constexpr void Bitboards::cast_magic()
{

}
