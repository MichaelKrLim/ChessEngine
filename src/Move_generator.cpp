#include "Bitboard.h"
#include "Board.h"
#include "Constants.h"
#include "Constants.h"
#include "Move_generator.h"
#include "Position.h"
#include "Utility.h"

#include <bit>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

namespace
{
	using namespace engine;

	constexpr static std::array<std::uint8_t, board_size*board_size> rook_relevant_bits = 
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

	constexpr static std::array<std::uint8_t, board_size*board_size> bishop_relevant_bits = 
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

	constexpr std::array<Position, 8> knight_moves_ =
	{{
		Position{2, 1}, Position{2, -1}, Position{-2, 1}, Position{-2, -1},
		Position{1, 2}, Position{1, -2}, Position{-1, 2}, Position{-1, -2}
	}};
	constexpr std::array<Position, 4> bishop_moves_ =
	{{
		Position{1,  1}, Position{1,  -1},
		Position{-1, 1}, Position{-1, -1}
	}};
	constexpr std::array<Position, 8> king_moves_ =
	{{
		Position{1, 0}, Position{-1, 0}, Position{0,  1}, Position{0,  -1},
		Position{1, 1}, Position{-1, 1}, Position{1, -1}, Position{-1, -1}
	}};
	constexpr std::array<Position, 4> rook_moves_ =
	{{
		Position{1, 0}, Position{0, 1}, Position{-1, 0}, Position{0, -1}
	}};

	struct Magic_square
	{
		std::vector<Bitboard> attack_table;
		std::uint64_t mask;
		std::uint64_t magic;
		std::uint8_t shift;
	};

	const std::uint64_t rook_mask(const Position& square)
	{
		std::uint64_t result{0ULL};
		for(int r = square.rank_+1; r <= 6; r++) result |= (1ULL << (square.file_ + r*8));
		for(int r = square.rank_-1; r >= 1; r--) result |= (1ULL << (square.file_ + r*8));
		for(int f = square.file_+1; f <= 6; f++) result |= (1ULL << (f + square.rank_*8));
		for(int f = square.file_-1; f >= 1; f--) result |= (1ULL << (f + square.rank_*8)); 
		return result;
	}

	const std::uint64_t bishop_mask(const Position& square)
	{
		std::uint64_t result{0ULL};
		for(int r = square.rank_+1, f = square.file_+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
		for(int r = square.rank_+1, f = square.file_-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
		for(int r = square.rank_-1, f = square.file_+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
		for(int r = square.rank_-1, f = square.file_-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
		return result;
	}

	std::vector<Bitboard> generate_blocker_configurations(const Position& square, const Piece& piece_type)
	{
		std::vector<Bitboard> blocker_configurations{};
		bool is_bishop = piece_type == Piece::bishop;
		const std::uint64_t attack_mask = is_bishop? bishop_mask(square) : rook_mask(square);
		for(std::uint64_t blockers = attack_mask; blockers != 0; blockers = (blockers - 1) & attack_mask) 
		{
			blocker_configurations.push_back(Bitboard{blockers});
		}
		return blocker_configurations;
	}

	const Bitboard rook_reachable_squares(const Position& rook_square, const Bitboard& occupied_squares)
	{
		Bitboard valid_moves{0ULL};
		for(const auto& [dr, df] : rook_moves_) 
		{
			const Position offset{dr, df};
			for(Position target_square{rook_square+offset}; is_on_board(target_square) && is_free(target_square, occupied_squares); target_square+=offset)
				valid_moves.add_piece(target_square);
		}
		return valid_moves;
	}

	const Bitboard bishop_reachable_squares(const Position& bishop_square, const Bitboard& occupied_squares)
	{
		Bitboard valid_moves{0ULL};
		for(const auto& [dr, df] : bishop_moves_) 
		{
			const Position offset{dr, df};
			for(Position target_square{bishop_square+offset}; is_on_board(target_square) && is_free(target_square, occupied_squares); target_square+=offset)
				valid_moves.add_piece(target_square);
		}
		return valid_moves;
	}

	const std::uint64_t random_uint64()
	{
		static std::random_device rd;
		static std::mt19937_64 gen(rd());
		return gen();
	}

	const std::uint64_t random_uint64_fewbits()
	{
		return random_uint64() & random_uint64() & random_uint64();
	}

	const std::uint64_t magic_hash(const Bitboard& key, const std::uint64_t& magic, int shift) 
	{
		return (key*magic) >> shift;
	}

	const Magic_square find_magic(const Position& square, const Piece& piece_type)
	{
		try
		{
			const auto square_index = to_index(square);
			const bool is_bishop = piece_type == Piece::bishop;
			const auto relevant_bits = is_bishop? bishop_relevant_bits[square_index] : rook_relevant_bits[square_index];
			const auto shift = 64-relevant_bits;
			const std::uint64_t mask = is_bishop? bishop_mask(square) : rook_mask(square);
			const auto& reachable_squares = is_bishop? bishop_reachable_squares : rook_reachable_squares;
			const std::vector<Bitboard> blocker_configurations = generate_blocker_configurations(square, piece_type);
			const std::vector<Bitboard> attacks = [&reachable_squares, &blocker_configurations, &square, &is_bishop]()
			{
				std::vector<Bitboard> attacks{};
				for(const auto& blocker_configuration : blocker_configurations)
				{
					attacks.push_back(reachable_squares(square, blocker_configuration));
				}
				return attacks;
			}();
			for(std::size_t k{0}; k < 100000000; ++k) 
			{
				std::vector<Bitboard> used(1 << relevant_bits, Bitboard{0xFFFFFFFFFFFFFFFFULL});
				const std::uint64_t magic = random_uint64_fewbits();
				if(std::popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
				bool fail{false};
				for(std::size_t i{0}; !fail && i < blocker_configurations.size(); ++i) 
				{
					const std::uint64_t magic_index = magic_hash(blocker_configurations[i], magic, shift);
					//if(!is_bishop && square == Position{7, 0} && !fail) 
					//std::cout << blocker_configurations[i] << "\n" 
					//<<  attacks[i]<< "\n" 
					//<< used[magic_index] << "\n" 
					//<< (int) (used[magic_index] != attacks[i]) << "\n";
					if(used[magic_index] == 0xFFFFFFFFFFFFFFFFULL) used[magic_index] = attacks[i];
					else if(used[magic_index] != attacks[i]) fail = true;
				}
				if(!fail) return Magic_square{used, mask, magic, std::uint8_t(64-relevant_bits)};
			}
			throw std::runtime_error("could not find magic in: find_magic(...)");
		}
		catch(std::runtime_error& r)
		{
			std::cout << "Move_generator failed initialisiation with: " << r.what();
			std::exit(EXIT_FAILURE);
		}
	}

	const std::array<Magic_square, 64> bishop_magic_squares_ = []()
	{
		std::array<Magic_square, 64> bishop_magic_squares{};
		for(std::size_t rank{0}; rank<board_size; ++rank)
		{
			for(std::size_t file{0}; file<board_size; ++file)
			{
				const Position current_square = Position{rank, file};
				bishop_magic_squares[to_index(current_square)] = find_magic(current_square, Piece::bishop);
			}
		}
		return bishop_magic_squares;
	}();

	const std::array<Magic_square, 64> rook_magic_squares_ = []()
	{
		std::array<Magic_square, 64> rook_magic_squares{};
		for(std::size_t rank{0}; rank<board_size; ++rank)
		{
			for(std::size_t file{0}; file<board_size; ++file)
			{
				const Position current_square = Position{rank, file};
				rook_magic_squares[to_index(current_square)] = find_magic(current_square, Piece::rook);
			}
		}
		return rook_magic_squares;
	}();

	[[maybe_unused]] const moves_type pawn_legal_moves(const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player)
	{
		moves_type legal_moves{};
		const auto rank = 0xFF;
		const auto pawn_direction = active_player == Side::white ? 1 : -1;
		const auto starting_rank  = active_player == Side::white ? 1 : 6;
		const auto promotion_rank = active_player == Side::white ? 7 : 0;
		const auto promotion_mask = rank << (promotion_rank*board_size);

		pawns_bb.for_each_piece([&](const Position& original_square)
		{
			Bitboard pawn_bb{1ULL << to_index(original_square)};
			Bitboard valid_moves{0};
			Bitboard single_moves = (pawn_bb << board_size*pawn_direction) & ~occupied_squares;
			valid_moves |= single_moves;

			const auto rank_mask = rank << ((starting_rank+pawn_direction)*board_size);
			Bitboard double_moves = (single_moves & rank_mask) << board_size*pawn_direction;
			valid_moves |= double_moves;

			Bitboard left_captures = (single_moves << -1) & occupied_squares & ~file_h;
			Bitboard right_captures = (single_moves << 1) & occupied_squares & ~file_a;
			valid_moves |= left_captures | right_captures;

			Bitboard promotion_moves = single_moves & promotion_mask;
			valid_moves |= promotion_moves;
			valid_moves.for_each_piece([&legal_moves, &original_square](const Position& destination_square)
			{
				legal_moves[original_square].push_back(destination_square);
			});
		});
		return legal_moves;
	}

	[[maybe_unused]] const moves_type knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares)
	{
		moves_type legal_moves{};
		knight_bb.for_each_piece([occupied_squares, &legal_moves](const Position& original_square)
		{
			for(const auto& move : knight_moves_)
			{
				const auto [del_rank, del_file] = move;
				const Position destination_square(original_square.rank_+del_rank, original_square.file_+del_file);
				if(is_valid_destination(destination_square, occupied_squares))
					legal_moves[original_square].push_back(destination_square);
			}
		});
		return legal_moves;
	}

	[[maybe_unused]] const moves_type king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares)
	{
		moves_type legal_moves{};
		const Position original_square = king_bb.lsb_index();
		for(const auto& move : king_moves_)
		{
			const auto [del_rank, del_file] = move;
			const Position destination_square(original_square.rank_+del_rank, original_square.file_+del_file);
			if(is_valid_destination(destination_square, occupied_squares))
				legal_moves[original_square].push_back(destination_square);
		}
		return legal_moves;
	}

	const Bitboard bishop_legal_moves_bb(const Position& original_square, const Bitboard& occupied_squares)
	{
		const auto square_index = to_index(original_square);
		const auto& magic_square = bishop_magic_squares_[square_index];
		const auto& attack_table = bishop_magic_squares_[square_index].attack_table;
		std::uint64_t magic_index = magic_hash(occupied_squares & magic_square.mask, magic_square.magic, magic_square.shift);
		return attack_table[magic_index];
	}

	const Bitboard rook_legal_moves_bb(const Position& original_square, const Bitboard& occupied_squares)
	{
		const auto square_index = to_index(original_square);
		const auto& magic_square = rook_magic_squares_[square_index];
		const auto& attack_table = rook_magic_squares_[square_index].attack_table;
		std::uint64_t magic_index = magic_hash(occupied_squares & magic_square.mask, magic_square.magic, magic_square.shift);
		return attack_table[magic_index];
	}

	[[maybe_unused]] const moves_type rook_legal_moves(const Bitboard& rook_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares)
	{
		moves_type legal_moves{};
		rook_bb.for_each_piece([&legal_moves, &current_sides_occupied_squares, occupied_squares](const Position& original_square)
		{
			(rook_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares).for_each_piece([&legal_moves, &original_square](const auto& destination_square)
			{
				legal_moves[original_square].push_back(destination_square);
			});
		});
		return legal_moves;
	}

	const moves_type bishop_legal_moves(const Bitboard& bishop_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares)
	{
		moves_type legal_moves{};
		bishop_bb.for_each_piece([&legal_moves, &current_sides_occupied_squares, occupied_squares](const Position& original_square)
		{
			(bishop_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares).for_each_piece([&legal_moves, &original_square](const auto& destination_square)
			{
				legal_moves[original_square].push_back(destination_square);
			});
		});
		return legal_moves;
	}

	[[maybe_unused]] const moves_type queen_legal_moves(const Bitboard& queen_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares)
	{
		moves_type legal_moves{};
		queen_bb.for_each_piece([&legal_moves, &current_sides_occupied_squares, occupied_squares](const Position& original_square)
		{
			auto queen_legal_moves_bb = bishop_legal_moves_bb(original_square, occupied_squares) | rook_legal_moves_bb(original_square, occupied_squares);
			(queen_legal_moves_bb & ~current_sides_occupied_squares).for_each_piece([&legal_moves, &original_square](const auto& destination_square)
			{
				legal_moves[original_square].push_back(destination_square);
			});
		});
		return legal_moves;
	}
}

namespace engine
{
	const std::array<moves_type, number_of_piece_types> legal_moves(const Board& board)
	{
		std::array<moves_type, number_of_piece_types> legal_moves{};
		const auto side_index = board.is_white_to_move? static_cast<std::uint8_t>(Side::white) : static_cast<std::uint8_t>(Side::black);
		const auto& pieces = board.sides[side_index].pieces;
		const auto& occupied_squares = board.occupied_squares;
		//legal_moves[static_cast<std::uint8_t>(Piece::pawn)]   = pawn_legal_moves(pieces[static_cast<std::size_t>(Piece::pawn)], occupied_squares, static_cast<Side>(side_index));
		//legal_moves[static_cast<std::uint8_t>(Piece::knight)] = knight_legal_moves(pieces[static_cast<std::size_t>(Piece::knight)], occupied_squares);
		//legal_moves[static_cast<std::uint8_t>(Piece::king)]   = king_legal_moves(pieces[static_cast<std::uint8_t>(Piece::king)], occupied_squares);

		legal_moves[static_cast<std::uint8_t>(Piece::bishop)]  = bishop_legal_moves(pieces[static_cast<std::uint8_t>(Piece::bishop)], occupied_squares, board.sides[side_index].occupied_squares);
		legal_moves[static_cast<std::uint8_t>(Piece::rook)]     = rook_legal_moves(pieces[static_cast<std::uint8_t>(Piece::rook)], occupied_squares, board.sides[side_index].occupied_squares);
		legal_moves[static_cast<std::uint8_t>(Piece::queen)]  = queen_legal_moves(pieces[static_cast<std::uint8_t>(Piece::queen)], occupied_squares, board.sides[side_index].occupied_squares);
		return legal_moves;
	}
}
