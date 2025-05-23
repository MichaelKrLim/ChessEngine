#include "Bitboard.h"
#include "State.h"
#include "Constants.h"
#include "Magic_util.h"
#include "Move_generator.h"
#include "Move.h"
#include "Position.h"
#include "Utility.h"
#include "Enum_map.h"

#include <array>
#include <functional>
#include <optional>
#include <utility>
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

	constexpr Bitboard knight_mask(const Position& square)
	{
		Bitboard reachable_squares{0ULL};
		for(const auto& move : knight_moves_)
		{
			const auto [del_rank, del_file] = move;
			const Position destination_square(square.rank_+del_rank, square.file_+del_file);
			if(is_on_board(destination_square))
				reachable_squares.add_piece(destination_square);
		}
		return reachable_squares;
	}

	constexpr Bitboard king_mask(const Position& square)
	{
		Bitboard reachable_squares{0ULL};
		for(const auto& move : king_moves_)
		{
			const auto [del_rank, del_file] = move;
			const Position destination_square(square.rank_+del_rank, square.file_+del_file);
			if(is_on_board(destination_square))
				reachable_squares.add_piece(destination_square);
		}
		return reachable_squares;
	}

	const void pawn_legal_moves(std::vector<Move>& legal_moves, const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player, const Bitboard& current_side_occupied_squares, const std::optional<Position>& en_passant_target_square, const Position& king_square = Position{-1, -1}, const Bitboard& pinned_pieces = {})
	{
		const auto rank_move = 8;
		const bool is_white = active_player == Side::white? true:false;
		const auto initial_rank = active_player == Side::white ? 1 : 6;
		const auto pawn_direction = is_white? 1 : -1;
		const auto promotion_rank = is_white? 7 : 0;
		const auto add_promotion_legal_move = [&legal_moves](const Position& origin_square, const Position& destination_square)
		{
			for(const auto& piece_type : all_promotion_pieces)
				legal_moves.push_back(Move{origin_square, destination_square, piece_type});
		};
		Bitboard single_moves = is_white? (pawns_bb << rank_move) : (pawns_bb >> rank_move);
		single_moves.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_};
			if(is_valid_destination(destination_square, occupied_squares) && (!pinned_pieces.is_occupied(origin_square) || origin_square.file_ == king_square.file_))
			{	
				if(destination_square.rank_ == promotion_rank)
					add_promotion_legal_move(origin_square, destination_square);
				else
					legal_moves.push_back(Move{origin_square, destination_square});
			}
			else
				single_moves.remove_piece(destination_square);
		});
		single_moves &= rank_bb(initial_rank+pawn_direction);
		const Bitboard double_moves = is_white? (single_moves << (rank_move)) : (single_moves >> (rank_move));
		double_moves.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction*2, destination_square.file_};
			if(is_valid_destination(destination_square, occupied_squares) && is_free(Position{destination_square.rank_-pawn_direction, destination_square.file_}, occupied_squares) && (!pinned_pieces.is_occupied(origin_square) || origin_square.file_ == king_square.file_))
				legal_moves.push_back(Move{origin_square, destination_square});
		});
		Bitboard pawns_to_move = pawns_bb & ~file_h;
		const Bitboard right_captures = is_white? (pawns_to_move << (rank_move+1)) : (pawns_to_move >> (rank_move-1));
		right_captures.for_each_piece([&](const Position& destination_square)
		{
			const bool can_capture = !is_free(destination_square, occupied_squares & ~current_side_occupied_squares) || destination_square == en_passant_target_square;
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_-1};
			const auto relative_diagonal = is_white? &Position::diagonal_index : &Position::antidiagonal_index;
			if(is_on_board(destination_square) && can_capture && (!pinned_pieces.is_occupied(origin_square) || std::invoke(relative_diagonal, &king_square) == std::invoke(relative_diagonal, &destination_square)))
			{
				if(destination_square.rank_ == promotion_rank)
					add_promotion_legal_move(origin_square, destination_square);
				else
					legal_moves.push_back(Move{origin_square, destination_square});
			}
		});
		pawns_to_move = pawns_bb & ~file_a;
		const auto left_captures = is_white? (pawns_to_move << (rank_move-1)) : (pawns_to_move >> (rank_move+1));
		left_captures.for_each_piece([&](const Position& destination_square)
		{
			const bool can_capture = !is_free(destination_square, occupied_squares & ~current_side_occupied_squares) || destination_square == en_passant_target_square;
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_+1};
			const auto relative_diagonal = is_white? &Position::antidiagonal_index : &Position::diagonal_index;
			if(is_on_board(destination_square) && can_capture && (!pinned_pieces.is_occupied(origin_square) || std::invoke(relative_diagonal, &king_square) == std::invoke(relative_diagonal, &destination_square)))
			{
				if(destination_square.rank_ == promotion_rank)
					add_promotion_legal_move(origin_square, destination_square);
				else
					legal_moves.push_back(Move{origin_square, destination_square});
			}
		});
	}

	const void knight_legal_moves(std::vector<Move>& legal_moves, const Bitboard& knight_bb, const Bitboard& our_occupied_squares)
	{
		knight_bb.for_each_piece([&](const auto& origin_square)
		{
			(knight_mask(origin_square) & ~our_occupied_squares).for_each_piece([&](const auto& destination_square)
			{
				legal_moves.push_back(Move{origin_square, destination_square});
			});
		});
	}

	const void king_legal_moves(std::vector<Move>& legal_moves, const Position& original_square, const Bitboard& occupied_squares, const Bitboard& our_occupied_squares, const bool is_white, const Castling_rights_map<bool>& castling_rights, const Bitboard& enemy_attack_map)
	{
		if(!(enemy_attack_map & (1ULL << to_index(original_square))))
		{
			if(castling_rights[Castling_rights::kingside])
			{
				Bitboard blocking_pieces_mask{is_white? 0x60ULL : 0x60ULL << (board_size*7)};
				if(!(occupied_squares & blocking_pieces_mask) && !(blocking_pieces_mask & enemy_attack_map))
					legal_moves.push_back(Move{original_square, Position{original_square.rank_, original_square.file_+2}});
			}
			if(castling_rights[Castling_rights::queenside])
			{
				Bitboard queenside_check_mask{is_white? 0xcULL : 0xcULL << (board_size*7)};
				Bitboard blocking_pieces_mask{is_white? 0xeULL : 0xeULL << (board_size*7)};
				if(!(occupied_squares & blocking_pieces_mask) && !(queenside_check_mask & enemy_attack_map))
					legal_moves.push_back(Move{original_square, Position{original_square.rank_, original_square.file_-2}});
			}
		}
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
		const auto& attack_table = magic_square.attack_table;
		std::uint64_t magic_index = magic_hash(occupied_squares & magic_square.mask, magic_square.magic, magic_square.shift);
		return attack_table[magic_index];
	}

	const Bitboard rook_legal_moves_bb(const Position& original_square, const Bitboard& occupied_squares)
	{
		const auto square_index = to_index(original_square);
		const auto& magic_square = rook_magic_squares_[square_index];
		const auto& attack_table = magic_square.attack_table;
		std::uint64_t magic_index = magic_hash(occupied_squares & magic_square.mask, magic_square.magic, magic_square.shift);
		return attack_table[magic_index];
	}

	const void rook_legal_moves(std::vector<Move>& legal_moves, const Bitboard& rook_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const Bitboard& pinned_pieces = {})
	{
		rook_bb.for_each_piece([&king_square, &legal_moves, &current_sides_occupied_squares, occupied_squares, &pinned_pieces](const Position& original_square)
		{
			(rook_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares).for_each_piece([&king_square, &legal_moves, &original_square, &pinned_pieces](const auto& destination_square)
			{
				const bool move_stays_pinned = (original_square.rank_ == king_square.rank_ && destination_square.rank_ == king_square.rank_) || (original_square.file_ == king_square.file_ && destination_square.file_ == king_square.file_);
				if(!pinned_pieces.is_occupied(original_square) || move_stays_pinned)
					legal_moves.push_back(Move{original_square, destination_square});
			});
		});
	}

	const void bishop_legal_moves(std::vector<Move>& legal_moves, const Bitboard& bishop_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const Bitboard& pinned_pieces = {})
	{
		bishop_bb.for_each_piece([&king_square, &legal_moves, &current_sides_occupied_squares, occupied_squares, &pinned_pieces](const Position& original_square)
		{
			(bishop_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares).for_each_piece([&king_square, &legal_moves, &original_square, &pinned_pieces](const auto& destination_square)
			{
				const bool move_stays_pinned = (original_square.diagonal_index() == king_square.diagonal_index() && destination_square.diagonal_index() == king_square.diagonal_index()) || (original_square.antidiagonal_index() == king_square.antidiagonal_index() && destination_square.antidiagonal_index() == king_square.antidiagonal_index());
				if(!pinned_pieces.is_occupied(original_square) || move_stays_pinned)
					legal_moves.push_back(Move{original_square, destination_square});
			});
		});
	}

	const void queen_legal_moves(std::vector<Move>& legal_moves, const Bitboard& queen_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const Bitboard& pinned_pieces = {})
	{
		bishop_legal_moves(legal_moves, queen_bb, occupied_squares, current_sides_occupied_squares, king_square, pinned_pieces);
		rook_legal_moves(legal_moves, queen_bb, occupied_squares, current_sides_occupied_squares, king_square, pinned_pieces);
	}

	[[nodiscard]] const Bitboard generate_pinned_pieces(const State& state, const Position& king_square) noexcept
	{
		Bitboard pinned_pieces{0ULL};
		const auto& side = state.sides[state.side_to_move];
		const auto& enemy_side = state.sides[other_side(state.side_to_move)];
		auto find = [&](const auto& moves, const Bitboard& our_occupied_squares, const Bitboard& sliding_piece)
		{
			for(const auto& move : moves)
			{
				std::optional<Position> found_pinnable_piece{std::nullopt};
				for(Position current_square{king_square+move}; is_on_board(current_square); current_square+=move)
				{
					if(found_pinnable_piece && sliding_piece.is_occupied(current_square))
					{
						pinned_pieces.add_piece(found_pinnable_piece.value());
						break;
					}
					if(enemy_side.occupied_squares().is_occupied(current_square))
						break;
					if(our_occupied_squares.is_occupied(current_square))
					{
						if(!found_pinnable_piece.has_value())
							found_pinnable_piece = current_square;
						else
							break;
					}
				}
			}
		};
		find(bishop_moves_, side.occupied_squares(), enemy_side.pieces[Piece::bishop] | enemy_side.pieces[Piece::queen]);
		find(rook_moves_, side.occupied_squares(), enemy_side.pieces[Piece::rook] | enemy_side.pieces[Piece::queen]);
		return pinned_pieces;
	}
}

namespace engine
{
	std::vector<Move> legal_moves(const State& state) noexcept
	{
		std::vector<Move> legal_moves{};
		const Side_position our_side = state.sides[state.side_to_move];
		const auto& pieces = our_side.pieces;
		const auto& our_occupied_squares = our_side.occupied_squares();
		const auto occupied_squares = state.occupied_squares();
		const bool in_check = state.in_check();
		const Position king_square = our_side.pieces[Piece::king].lsb_square();
		legal_moves.reserve(max_legal_moves);
		const Bitboard pinned_pieces = generate_pinned_pieces(state, king_square);
		pawn_legal_moves(legal_moves, pieces[Piece::pawn], occupied_squares, state.side_to_move, our_occupied_squares, state.en_passant_target_square, king_square, pinned_pieces);
		knight_legal_moves(legal_moves, pieces[Piece::knight] & ~pinned_pieces, our_occupied_squares );

		bishop_legal_moves(legal_moves, pieces[Piece::bishop], occupied_squares, our_occupied_squares, king_square, pinned_pieces);
		rook_legal_moves(legal_moves, pieces[Piece::rook], occupied_squares, our_occupied_squares, king_square,  pinned_pieces);
		queen_legal_moves(legal_moves, pieces[Piece::queen], occupied_squares, our_occupied_squares, king_square, pinned_pieces);
		if(in_check)
		{
			const Side_position& enemy_side = state.sides[other_side(state.side_to_move)];
			Bitboard checking_pieces;
			auto find_checking_pieces = [&](const auto& pinning_bb, const auto& moves)
			{
				for(const auto& move : moves)
				{
					for(Position current_square{king_square+move}; is_on_board(current_square); current_square+=move)
					{
						if(pinning_bb.is_occupied(current_square))
						{
							checking_pieces.add_piece(current_square);
							break;
						}
						else if(occupied_squares.is_occupied(current_square))
							break;
					}
				}
			};
			find_checking_pieces(enemy_side.pieces[Piece::rook] | enemy_side.pieces[Piece::queen], rook_moves_);
			find_checking_pieces(enemy_side.pieces[Piece::bishop] | enemy_side.pieces[Piece::queen], bishop_moves_);
			const Bitboard attacking_knights = knight_mask(king_square) & enemy_side.pieces[Piece::knight];
			const Bitboard& our_king_bb = pieces[Piece::king];
			const Bitboard left_attacking_pawn = state.side_to_move == Side::white? (our_king_bb & ~file_h) << (board_size+1) : (our_king_bb & ~file_h) >> (board_size-1);
			const Bitboard right_attacking_pawn = state.side_to_move == Side::white? (our_king_bb & ~file_a) << (board_size-1) : (our_king_bb & ~file_a) >> (board_size+1);
			const Bitboard attacking_pawns = enemy_side.pieces[Piece::pawn] & (left_attacking_pawn | right_attacking_pawn);
			const auto knight_popcount = attacking_knights.popcount(), pawn_popcount = attacking_pawns.popcount();
			if(checking_pieces.popcount() == 1 && knight_popcount == 0 && pawn_popcount == 0)
			{
				std::erase_if(legal_moves, [&](const Move& move)
				{
					const Position attacking_piece = checking_pieces.lsb_square();
					const Position destination_square = move.destination_square();
					if(king_square.rank_ == attacking_piece.rank_)
						return !(destination_square.rank_ == king_square.rank_ && destination_square.file_ >= std::min(king_square.file_, attacking_piece.file_) && destination_square.file_ <= std::max(king_square.file_, attacking_piece.file_));
					if(king_square.file_ == attacking_piece.file_)
						return !(destination_square.file_ == attacking_piece.file_ && destination_square.rank_ >= std::min(king_square.rank_, attacking_piece.rank_) && destination_square.rank_ <= std::max(king_square.rank_, attacking_piece.rank_));
					if(king_square.diagonal_index() == attacking_piece.diagonal_index())
						return !(destination_square.diagonal_index() == king_square.diagonal_index() && destination_square.rank_ >= std::min(king_square.rank_, attacking_piece.rank_) && destination_square.rank_ <= std::max(king_square.rank_, attacking_piece.rank_));
					if(king_square.antidiagonal_index() == attacking_piece.antidiagonal_index())
						return !(destination_square.antidiagonal_index() == king_square.antidiagonal_index() && destination_square.rank_ >= std::min(king_square.rank_, attacking_piece.rank_) && destination_square.rank_ <= std::max(king_square.rank_, attacking_piece.rank_));
					std::unreachable();
				});
			}
			else if(checking_pieces.popcount() < 1 && (knight_popcount == 1 || pawn_popcount == 1))
			{
				const Bitboard checking_piece = knight_popcount == 1? attacking_knights : attacking_pawns;
				std::erase_if(legal_moves, [&](const Move& move)
				{
					return move.destination_square() != checking_piece.lsb_square();
				});
			}
			else
				legal_moves.clear();
		}
		king_legal_moves(legal_moves, king_square, occupied_squares, our_occupied_squares | state.enemy_attack_map, state.side_to_move == Side::white? true : false, our_side.castling_rights, state.enemy_attack_map);
		return legal_moves;
	}

	Bitboard generate_attack_map(const State& state) noexcept
	{
		std::vector<Move> legal_moves{};
		Bitboard attack_map;
		const auto& pieces = state.sides[state.side_to_move].pieces;
		const auto& pawn_bb = pieces[Piece::pawn];
		const bool is_white = state.side_to_move == Side::white;
		auto occupied_squares = state.occupied_squares();
		occupied_squares.remove_piece(state.sides[other_side(state.side_to_move)].pieces[Piece::king].lsb_square());
		legal_moves.reserve(max_legal_moves);
		attack_map |= is_white? ((pawn_bb & ~file_a) << (board_size-1)) : ((pawn_bb & ~file_a) >> (board_size+1));
		attack_map |= is_white? ((pawn_bb & ~file_h) << (board_size+1)) : ((pawn_bb & ~file_h) >> (board_size-1));
		attack_map |= king_mask(pieces[Piece::king].lsb_square());
		knight_legal_moves(legal_moves, pieces[Piece::knight], Bitboard{0ULL});
		bishop_legal_moves(legal_moves, pieces[Piece::bishop], occupied_squares, Bitboard{0ULL}, {});
		rook_legal_moves(legal_moves, pieces[Piece::rook], occupied_squares, Bitboard{0ULL}, {});
		queen_legal_moves(legal_moves, pieces[Piece::queen], occupied_squares, Bitboard{0ULL}, {});
		for(const auto& move : legal_moves)
		{
			attack_map.add_piece(move.destination_square());
		}
		return attack_map;
	}

	std::vector<Move> noisy_moves(const State& state) noexcept
	{
		// inefficient but that is okay also does not find checking moves
		std::vector<Move> moves = legal_moves(state);
		std::erase_if(moves, [&state](const Move& move)
		{
			return is_free(move.destination_square(), state.sides[other_side(state.side_to_move)].occupied_squares());
		});
		return moves;
	}
}
