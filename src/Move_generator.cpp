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

	struct Magic_square
	{
		const std::vector<Bitboard>* attack_table;
		std::uint64_t mask;
		std::uint64_t magic;
		std::uint8_t shift;
	};

	static std::array<Magic_square, 64> bishop_magic_squares_{};
	static std::array<Magic_square, 64> rook_magic_squares_{};
	static std::vector<std::vector<Bitboard>> attack_table_{};

	constexpr static std::array<std::uint8_t, board_size*board_size> rook_rellevant_bits = 
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

	constexpr static std::array<std::uint8_t, board_size*board_size> bishop_rellevant_bits = 
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

	std::vector<Bitboard> blocker_configurations(const Position& square, const bool& is_bishop)
	{
		std::vector<Bitboard> blocker_configurations{};
		std::array<Position, 4> rook_moves;
		if(!is_bishop)
			rook_moves = {Position{0, 1}, Position{1, 0}, Position{-1, 0}, Position{0, -1}};
		const std::array<Position, 4>& directions = is_bishop? bishop_moves_ : rook_moves;
		std::array<std::vector<Position>, 4> rays{};
		for(std::uint8_t direction{0}; direction < 4; ++direction)
		{
			for(std::uint8_t offset{1}; ; ++offset)
			{
				Position blocker_position = square + Position{directions[direction].rank_*offset, directions[direction].file_*offset};
				if(!is_on_board(blocker_position)) break;
				else rays[direction].push_back(blocker_position);
			}
		}
		const auto generate_configurations = [&blocker_configurations, &rays](this auto&& rec, Bitboard current_configuration, size_t ray_index) -> void
		{
			if(ray_index == 4)
			{
				blocker_configurations.push_back(current_configuration);
				return;
			}
			
			if(rays[ray_index].empty())
			{
				rec(current_configuration, ray_index+1);
				return;
			}
			
			for(const auto& target_square : rays[ray_index])
			{
				Bitboard new_configuration = current_configuration;
				new_configuration.add_piece(to_index(target_square));
				rec(new_configuration, ray_index+1);
			}
		};
		
		generate_configurations(Bitboard{0ULL}, 0);
		return blocker_configurations;
	}

	const Bitboard rook_reachable_squares(const Position& original_square, const Bitboard& occupied_squares)
	{
		Bitboard valid_moves{};
		for(int del_rank{0}; del_rank < board_size; ++del_rank)
		{
			for(int del_file{0}; del_file < board_size; ++del_file)
			{
				const Position destination_square = original_square + Position{del_rank, del_file};
				if(is_valid_destination(destination_square, occupied_squares))
					valid_moves.add_piece(to_index(destination_square));
				else 
					return valid_moves;
			}
		}
		return valid_moves;
	}

	const Bitboard bishop_reachable_squares(const Position& bishop_square, const Bitboard& occupied_squares)
	{
		Bitboard valid_moves{0ULL};
		for(const auto& [dr, df] : bishop_moves_) 
		{
			const Position offset{dr, df};
			for(Position current_move{bishop_square+offset}; is_on_board(current_move) && !(occupied_squares & (1ULL << to_index(current_move))); current_move+=offset)
				valid_moves |= (1ULL << to_index(current_move));
		}
		return valid_moves;
	}

	std::vector<std::vector<Bitboard>> create_attack_tables()
	{
		std::vector<std::vector<Bitboard>> attack_table{};
		for(std::size_t rank{0}; rank<board_size; ++rank)
		{
			for(std::size_t file{0}; file<board_size; ++file)
			{
				const Position current_square = Position{rank, file};
				const auto current_index = to_index(current_square);
				std::vector<Bitboard> current_moves{};
				for(const auto& blocker_configuration : blocker_configurations(current_square, false))
					current_moves.push_back(rook_reachable_squares(current_square, blocker_configuration));
				attack_table.push_back(current_moves);
				rook_magic_squares_[current_index].attack_table = &attack_table.back();
			}
		}
		for(std::size_t rank{0}; rank<board_size; ++rank)
		{
			for(std::size_t file{0}; file<board_size; ++file)
			{
				const Position current_square = Position{rank, file};
				const std::size_t current_index = to_index(current_square);
				std::vector<Bitboard> current_moves{};
				for(const auto& blocker_configuration : blocker_configurations(current_square, true))
					current_moves.push_back(bishop_reachable_squares(current_square, blocker_configuration));
				attack_table.push_back(current_moves);
				bishop_magic_squares_[current_index].attack_table = &attack_table.back();
			}
		}
		return attack_table;
	}

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

	const std::size_t magic_hash(const Bitboard& key, const std::uint64_t& magic, int rellevant_bits) 
	{
		return (key*magic) >> (64-rellevant_bits);
	}

	const Magic_square find_magic(const Position& square, std::uint64_t mask, const std::vector<Bitboard>& blocker_configurations, const std::uint8_t rellevant_bits, const std::vector<Bitboard>& attack_table)
	{
		if(attack_table.size() > 1000)
			for(const auto& el : attack_table) std::cerr << el << "\n";
		const std::size_t n = std::popcount(mask);
		try
		{
			for(std::size_t k{0}; k < 100000000; ++k) 
			{
				//std::cerr << k << "\n"; //////////////////////////////////////////////////////////
				std::vector<Bitboard> used(1 << n);
				const std::uint64_t magic = random_uint64_fewbits();
				if(std::popcount((mask * magic) & 0xFF00000000000000ULL) < 6)
					continue;
				bool fail{false};
				for(std::size_t i{0}; !fail && i < attack_table.size(); ++i) 
				{
					std::cerr << attack_table.size() << ' ' << i << ' ';
					const std::size_t magic_index = magic_hash(blocker_configurations[i], magic, rellevant_bits);
					if(used[magic_index] == 0ULL) used[magic_index] = attack_table[i];
					else if(used[magic_index] != attack_table[i]) fail = true;
				} std::cerr << "we made it!!";
				if(!fail) return Magic_square{&attack_table, mask, magic, static_cast<std::uint8_t>(64-rellevant_bits)};
			}
			throw std::runtime_error("could not find magic find_magic(...)");
		}
		catch(std::runtime_error& r)
		{
			std::cout << "Move_generator failed initialisiation with: " << r.what();
			std::exit(EXIT_FAILURE);
		}
	}

	void cast_magic()
	{
		for(std::size_t rank{0}; rank < board_size; ++rank)
		{
			for(std::size_t file{0}; file < board_size; ++file)
			{
				const Position current_square{rank, file};
				const std::size_t current_index{to_index(current_square)};
				bishop_magic_squares_[current_index] = find_magic(current_square, bishop_mask(current_square), blocker_configurations(current_square, true),  bishop_rellevant_bits[to_index(current_square)], *bishop_magic_squares_[current_index].attack_table);
				for(const auto& el : *bishop_magic_squares_[current_index].attack_table) std::cerr << el << "\n";
				rook_magic_squares_[current_index]   = find_magic(current_square, rook_mask(current_square),   blocker_configurations(current_square, false), rook_rellevant_bits[to_index(current_square)],   *rook_magic_squares_[current_index].attack_table);
				std::cerr << "found magics for index " << current_square << "\n";
			}
		}
	}

	const moves_type pawn_legal_moves(const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player)
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

	const moves_type knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares)
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

	const moves_type king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares)
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
		const std::uint64_t square_index = to_index(original_square);
		std::uint64_t magic_index{occupied_squares};
		const std::vector<Bitboard>* attack_table = bishop_magic_squares_[square_index].attack_table;
		magic_index 							 &= bishop_magic_squares_[square_index].mask;
		magic_index 							 *= bishop_magic_squares_[square_index].magic;
		magic_index 							>>= bishop_magic_squares_[square_index].shift;
		return (*attack_table)[magic_index];
	}

	const Bitboard rook_legal_moves_bb(const Position& original_square, const Bitboard& occupied_squares)
	{
		const std::uint64_t square_index = to_index(original_square);
		std::uint64_t magic_index{occupied_squares};
		const std::vector<Bitboard>* attack_table = rook_magic_squares_[square_index].attack_table;
		magic_index 							 &= rook_magic_squares_[square_index].mask;
		magic_index 							 *= rook_magic_squares_[square_index].magic;
		magic_index 							>>= rook_magic_squares_[square_index].shift;
		return (*attack_table)[magic_index];
	}

	const moves_type rook_legal_moves(const Bitboard& rook_bb, const Bitboard& occupied_squares)
	{
		moves_type legal_moves{};
		rook_bb.for_each_piece([&legal_moves, occupied_squares](const Position& original_square)
		{
			rook_legal_moves_bb(original_square, occupied_squares).for_each_piece([&legal_moves, &original_square](const auto& destination_square)
			{
				legal_moves[original_square].push_back(destination_square);
			});
		});
		return legal_moves;
	}

	const moves_type bishop_legal_moves(const Bitboard& bishop_bb, const Bitboard& occupied_squares)
	{
		moves_type legal_moves{};
		bishop_bb.for_each_piece([&legal_moves, occupied_squares](const Position& original_square)
		{
			bishop_legal_moves_bb(original_square, occupied_squares).for_each_piece([&legal_moves, &original_square](const auto& destination_square)
			{
				legal_moves[original_square].push_back(destination_square);
			});
		});
		return legal_moves;
	}

	const moves_type queen_legal_moves(const Bitboard& queen_bb, const Bitboard& occupied_squares)
	{
		moves_type legal_moves{};
		queen_bb.for_each_piece([&legal_moves, occupied_squares](const Position& original_square)
		{
			(bishop_legal_moves_bb(original_square, occupied_squares) | rook_legal_moves_bb(original_square, occupied_squares)).for_each_piece([&legal_moves, &original_square](const auto& destination_square)
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
		const auto& occupied_squares = board.sides[side_index].occupied_squares;
		legal_moves[static_cast<std::uint8_t>(Piece::pawn)]   = pawn_legal_moves(pieces[static_cast<std::size_t>(Piece::pawn)], occupied_squares, static_cast<Side>(side_index));
		legal_moves[static_cast<std::uint8_t>(Piece::knight)] = knight_legal_moves(pieces[static_cast<std::size_t>(Piece::knight)], occupied_squares);
		legal_moves[static_cast<std::uint8_t>(Piece::king)]   = king_legal_moves(pieces[static_cast<std::uint8_t>(Piece::king)], occupied_squares);

		legal_moves[static_cast<std::uint8_t>(Piece::bishop)] = bishop_legal_moves(pieces[static_cast<std::uint8_t>(Piece::bishop)], occupied_squares);
		legal_moves[static_cast<std::uint8_t>(Piece::rook)]   = rook_legal_moves(pieces[static_cast<std::uint8_t>(Piece::rook)], occupied_squares);
		legal_moves[static_cast<std::uint8_t>(Piece::queen)]  = queen_legal_moves(pieces[static_cast<std::uint8_t>(Piece::queen)], occupied_squares);
		return legal_moves;
	}

	void initialise_move_generator()
	{
		create_attack_tables();
		cast_magic();
	}
}
