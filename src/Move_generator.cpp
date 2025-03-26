#include "Bitboard.h"
#include "Board.h"
#include "Constants.h"
#include "Magic_util.h"
#include "Move_generator.h"
#include "Move.h"
#include "Position.h"
#include "Utility.h"

#include <array>
#include <optional>
#include <unordered_set>
#include <vector>

namespace
{
	using namespace engine;
	#include "magic_squares.h"

	constexpr std::array<Position, 8> knight_moves_ =
	{{
		Position{2, 1}, Position{2, -1}, Position{-2, 1}, Position{-2, -1},
		Position{1, 2}, Position{1, -2}, Position{-1, 2}, Position{-1, -2}
	}};

	constexpr std::array<Position, 8> king_moves_ =
	{{
		Position{1, 0}, Position{-1, 0}, Position{0,  1}, Position{0,  -1},
		Position{1, 1}, Position{-1, 1}, Position{1, -1}, Position{-1, -1}
	}};

	const void pawn_legal_moves(std::vector<Move>& legal_moves, const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player, const Bitboard& current_side_occupied_squares, const std::unordered_set<Position>& pinned_pieces = {})
	{
		const auto rank_move = 8;
		const bool is_white = active_player == Side::white? true:false;
		const auto initial_rank = active_player == Side::white ? 1 : 6;
		const auto pawn_direction = is_white? 1 : -1;
		const Bitboard single_moves = is_white? (pawns_bb << rank_move) : (pawns_bb >> rank_move);
		single_moves.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_};
			if(is_valid_destination(destination_square, occupied_squares) && !pinned_pieces.contains(origin_square))
				legal_moves.push_back(Move{origin_square, destination_square});
		});
		Bitboard pawns_to_move = pawns_bb & rank_bb(initial_rank);
		const Bitboard double_moves = is_white? (pawns_to_move << (rank_move*2)) : (pawns_to_move >> (rank_move*2));
		double_moves.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction*2, destination_square.file_};
			if(is_valid_destination(destination_square, occupied_squares) && is_free(Position{destination_square.rank_-pawn_direction, destination_square.file_}, occupied_squares) && !pinned_pieces.contains(origin_square))
				legal_moves.push_back(Move{origin_square, destination_square});
		});
		pawns_to_move = pawns_bb & ~file_h;
		const Bitboard right_captures = is_white? (pawns_to_move << (rank_move+1)) : (pawns_to_move >> (rank_move-1));
		right_captures.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_-1};
			if(is_on_board(destination_square) && !is_free(destination_square, occupied_squares & ~current_side_occupied_squares) && !pinned_pieces.contains(origin_square))
				legal_moves.push_back(Move{origin_square, destination_square});
		});
		pawns_to_move = pawns_bb & ~file_a;
		const auto left_captures = is_white? (pawns_to_move << (rank_move-1)) : (pawns_to_move >> (rank_move+1));
		left_captures.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_+1};
			if(is_on_board(destination_square) && !is_free(destination_square, occupied_squares & ~current_side_occupied_squares) && !pinned_pieces.contains(origin_square))
				legal_moves.push_back(Move{origin_square, destination_square});
		});
	}

	const void knight_legal_moves(std::vector<Move>& legal_moves, const Bitboard& knight_bb, const Bitboard& our_occupied_squares, const std::unordered_set<Position>& pinned_pieces = {})
	{
		knight_bb.for_each_piece([&](const Position& original_square)
		{
			if(!pinned_pieces.contains(original_square))
			{
				for(const auto& move : knight_moves_)
				{
					const auto [del_rank, del_file] = move;
					const Position destination_square(original_square.rank_+del_rank, original_square.file_+del_file);
					if(is_valid_destination(destination_square, our_occupied_squares))
						legal_moves.push_back(Move{original_square, destination_square});
				}
			}
		});
	}

	const void king_legal_moves(std::vector<Move>& legal_moves, const Bitboard& king_bb, const Bitboard& our_occupied_squares, const std::unordered_set<Position>& pinned_pieces = {})
	{
		const Position original_square = king_bb.lsb_square();
		for(const auto& move : king_moves_)
		{
			const auto [del_rank, del_file] = move;
			const Position destination_square(original_square.rank_+del_rank, original_square.file_+del_file);
			if(is_valid_destination(destination_square, our_occupied_squares))
				legal_moves.push_back(Move{original_square, destination_square});
		}
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

	const void rook_legal_moves(std::vector<Move>& legal_moves, const Bitboard& rook_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const std::unordered_set<Position>& pinned_pieces = {})
	{
		rook_bb.for_each_piece([&legal_moves, &current_sides_occupied_squares, occupied_squares, &pinned_pieces](const Position& original_square)
		{
			(rook_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares).for_each_piece([&legal_moves, &original_square, &pinned_pieces](const auto& destination_square)
			{
				if(!pinned_pieces.contains(original_square))
				legal_moves.push_back(Move{original_square, destination_square});
			});
		});
	}

	const void bishop_legal_moves(std::vector<Move>& legal_moves, const Bitboard& bishop_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const std::unordered_set<Position>& pinned_pieces = {})
	{
		bishop_bb.for_each_piece([&legal_moves, &current_sides_occupied_squares, occupied_squares, &pinned_pieces](const Position& original_square)
		{
			(bishop_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares).for_each_piece([&legal_moves, &original_square, &pinned_pieces](const auto& destination_square)
			{
				if(!pinned_pieces.contains(original_square))
					legal_moves.push_back(Move{original_square, destination_square});
			});
		});
	}

	const void queen_legal_moves(std::vector<Move>& legal_moves, const Bitboard& queen_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const std::unordered_set<Position>& pinned_pieces = std::unordered_set<Position>())
	{
		queen_bb.for_each_piece([&legal_moves, &current_sides_occupied_squares, occupied_squares, &pinned_pieces](const Position& original_square)
		{
			auto queen_legal_moves_bb = bishop_legal_moves_bb(original_square, occupied_squares) | rook_legal_moves_bb(original_square, occupied_squares);
			(queen_legal_moves_bb & ~current_sides_occupied_squares).for_each_piece([&legal_moves, &original_square, &pinned_pieces](const auto& destination_square)
			{
				if(!pinned_pieces.contains(original_square))
					legal_moves.push_back(Move{original_square, destination_square});
			});
		});
	}

	[[nodiscard]] const std::unordered_set<Position> generate_pinned_pieces(const Board& board) noexcept
	{
		std::unordered_set<Position> pinned_pieces;
		const auto& side = board.sides[static_cast<std::uint8_t>(board.side_to_move)];
		const auto& enemy_side = board.sides[static_cast<std::uint8_t>(!board.side_to_move)];
		const Bitboard enemy_sliding_pieces = enemy_side.pieces[static_cast<std::uint8_t>(Piece::rook)] | enemy_side.pieces[static_cast<std::uint8_t>(Piece::bishop)] | enemy_side.pieces[static_cast<std::uint8_t>(Piece::queen)];
		const Position king_square = side.pieces[static_cast<std::uint8_t>(Piece::king)].lsb_square();
		const auto sliding_moves = { Position{1,  1}, Position{1,  -1}, Position{-1, 1}, Position{-1, -1},
									 Position{1, 0},  Position{0, 1},   Position{-1, 0}, Position{0, -1} };
		for(const auto& move : sliding_moves)
		{
			std::optional<Position> found_pinnable_piece{std::nullopt};
			for(Position current_square{king_square+move}; is_on_board(current_square); current_square+=move)
			{
				if(found_pinnable_piece && enemy_sliding_pieces.is_occupied(current_square))
				{
					pinned_pieces.insert(found_pinnable_piece.value());
					break;
				}
				if(enemy_side.occupied_squares().is_occupied(current_square))
					break;
				if(side.occupied_squares().is_occupied(current_square))
				{
					if(!found_pinnable_piece.has_value())
						found_pinnable_piece = current_square;
					else
						break;
				}
			}
		}
		return pinned_pieces;
	}
}

namespace engine
{
	const std::vector<Move> legal_moves(const Board& board) noexcept
	{
		std::vector<Move> legal_moves{};
		const auto side_index = static_cast<std::uint8_t>(board.side_to_move);
		const auto& pieces = board.sides[side_index].pieces;
		const auto occupied_squares = board.occupied_squares();
		const bool in_check = board.in_check();
		if(!in_check)
		{
			legal_moves.reserve(max_legal_moves);
			const std::unordered_set<Position> pinned_pieces = generate_pinned_pieces(board);
			pawn_legal_moves(legal_moves, pieces[static_cast<std::size_t>(Piece::pawn)], occupied_squares, static_cast<Side>(side_index), board.sides[side_index].occupied_squares(), pinned_pieces);
			knight_legal_moves(legal_moves, pieces[static_cast<std::size_t>(Piece::knight)], board.sides[side_index].occupied_squares(), pinned_pieces);
			king_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::king)], board.sides[side_index].occupied_squares() | board.enemy_attack_map, pinned_pieces);

			bishop_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::bishop)], occupied_squares, board.sides[side_index].occupied_squares(), pinned_pieces);
			rook_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::rook)], occupied_squares, board.sides[side_index].occupied_squares(), pinned_pieces);
			queen_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::queen)], occupied_squares, board.sides[side_index].occupied_squares(), pinned_pieces);
		}
		else
		{
			legal_moves.reserve(king_max_adjacent_squares);
			king_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::king)], board.sides[side_index].occupied_squares() | board.enemy_attack_map);
			//TODO deal with double checks and finding pieces that can block check
		}
		return legal_moves;
	}

	const Bitboard generate_attack_map(const Board& board) noexcept
	{
		std::vector<Move> legal_moves{};
		Bitboard attack_map;
		const auto side_index = static_cast<std::uint8_t>(board.side_to_move);
		const auto& pieces = board.sides[side_index].pieces;
		const auto occupied_squares = board.occupied_squares();
		legal_moves.reserve(max_legal_moves);
		pawn_legal_moves(legal_moves, pieces[static_cast<std::size_t>(Piece::pawn)], occupied_squares, static_cast<Side>(side_index), board.sides[side_index].occupied_squares());
		knight_legal_moves(legal_moves, pieces[static_cast<std::size_t>(Piece::knight)], board.sides[side_index].occupied_squares());
		king_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::king)], board.sides[side_index].occupied_squares());

		bishop_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::bishop)], occupied_squares, board.sides[side_index].occupied_squares());
		rook_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::rook)], occupied_squares, board.sides[side_index].occupied_squares());
		queen_legal_moves(legal_moves, pieces[static_cast<std::uint8_t>(Piece::queen)], occupied_squares, board.sides[side_index].occupied_squares());
		for(const auto& move : legal_moves)
		{
			attack_map.add_piece(move.destination_square());
		}
		return attack_map;
	}
}
