#include "Bitboard.h"
#include "Constants.h"
#include "Enum_map.h"
#include "Fixed_capacity_vector.h"
#include "Magic_util.h"
#include "Move_generator.h"
#include "Move.h"
#include "Position.h"
#include "State.h"
#include "Utility.h"

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

	using fixed_vector_t = Fixed_capacity_vector<Move, max_legal_moves>;

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

	template <Moves_type moves_type, typename T>
	void pawn_moves(fixed_vector_t& legal_moves, const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player, const Bitboard& current_side_occupied_squares, const std::optional<Position>& en_passant_target_square, const T& satisfies_check_requirements, const Position& king_square = Position{-1, -1}, const Bitboard& pinned_pieces = {})
	{
		const auto rank_move = 8;
		const bool is_white = active_player == Side::white? true:false;
		const auto initial_rank = active_player == Side::white ? 1 : 6;
		const auto pawn_direction = is_white? 1 : -1;
		const auto promotion_rank = is_white? 7 : 0;
		const auto enemy_occupied_squares = occupied_squares & ~current_side_occupied_squares;
		const auto add_promotion_legal_move = [&legal_moves](const Position& origin_square, const Position& destination_square)
		{
			for(const auto& piece_type : all_promotion_pieces)
				legal_moves.push_back(Move{origin_square, destination_square, piece_type});
		};
		const auto add_move = [&promotion_rank, &add_promotion_legal_move, &legal_moves](const Position& origin_square, const Position& destination_square, const Move& current_move)
		{
			if(destination_square.rank_ == promotion_rank)
				add_promotion_legal_move(origin_square, destination_square);
			else
				legal_moves.push_back(current_move);
		};
		Bitboard single_moves = is_white? (pawns_bb << rank_move) : (pawns_bb >> rank_move);
		single_moves.for_each_piece([&](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_};
			const Move current_move{origin_square, destination_square};
			if(is_valid_destination(destination_square, occupied_squares) && (!pinned_pieces.is_occupied(origin_square) || origin_square.file_ == king_square.file_))
			{
				if constexpr (moves_type == Moves_type::noisy)
				{
					if(destination_square.rank_ == promotion_rank && satisfies_check_requirements(current_move))
						add_promotion_legal_move(origin_square, destination_square);
				}
				else if constexpr (moves_type == Moves_type::legal)
				{
					if(satisfies_check_requirements(current_move))
						add_move(origin_square, destination_square, current_move);
				}
			}
			else
				single_moves.remove_piece(destination_square);
		});
		if constexpr (moves_type == Moves_type::legal)
		{
			single_moves &= rank_bb(initial_rank+pawn_direction);
			const Bitboard double_moves = is_white? (single_moves << (rank_move)) : (single_moves >> (rank_move));
			double_moves.for_each_piece([&legal_moves, &occupied_squares, &pawn_direction, &satisfies_check_requirements, &pinned_pieces, &king_square](const Position& destination_square)
			{
				const Position origin_square = Position{destination_square.rank_-pawn_direction*2, destination_square.file_};
				const Move current_move{origin_square, destination_square};
				if(is_valid_destination(destination_square, occupied_squares) && (!pinned_pieces.is_occupied(origin_square) || origin_square.file_ == king_square.file_) && satisfies_check_requirements(current_move))
					legal_moves.push_back(current_move);
			});
		}
		Bitboard pawns_to_move = pawns_bb & ~file_h;
		const Bitboard right_captures = is_white? (pawns_to_move << (rank_move+1)) : (pawns_to_move >> (rank_move-1));
		auto capture_mask = enemy_occupied_squares;
		if(en_passant_target_square && right_captures.is_occupied(en_passant_target_square.value()))
			capture_mask.add_piece(en_passant_target_square.value());
		(right_captures & capture_mask).for_each_piece([&add_move, &pinned_pieces, &king_square, &occupied_squares, &pawn_direction, &current_side_occupied_squares, &is_white, &satisfies_check_requirements, &en_passant_target_square](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_-1};
			const auto relative_diagonal = is_white? &Position::diagonal_index : &Position::antidiagonal_index;
			const Move current_move{origin_square, destination_square};
			if(is_on_board(destination_square) && (!pinned_pieces.is_occupied(origin_square) || std::invoke(relative_diagonal, &king_square) == std::invoke(relative_diagonal, &destination_square)) && satisfies_check_requirements(current_move))
				add_move(origin_square, destination_square, current_move);
		});
		pawns_to_move = pawns_bb & ~file_a;
		const auto left_captures = is_white? (pawns_to_move << (rank_move-1)) : (pawns_to_move >> (rank_move+1));
		if(en_passant_target_square)
		{
			if(capture_mask.is_occupied(en_passant_target_square.value()))
				capture_mask.remove_piece(en_passant_target_square.value());
			else if(left_captures.is_occupied(en_passant_target_square.value()))
				capture_mask.add_piece(en_passant_target_square.value());
		}
		(left_captures & capture_mask).for_each_piece([&add_move, &pinned_pieces, &occupied_squares, &pawn_direction, &is_white, &current_side_occupied_squares, &en_passant_target_square, &king_square, &satisfies_check_requirements](const Position& destination_square)
		{
			const Position origin_square = Position{destination_square.rank_-pawn_direction, destination_square.file_+1};
			const auto relative_diagonal = is_white? &Position::antidiagonal_index : &Position::diagonal_index;
			const Move current_move{origin_square, destination_square};
			if(is_on_board(destination_square) && (!pinned_pieces.is_occupied(origin_square) || std::invoke(relative_diagonal, &king_square) == std::invoke(relative_diagonal, &destination_square)) && satisfies_check_requirements(current_move))
				add_move(origin_square, destination_square, current_move);
		});
	}

	template <Moves_type moves_type>
	void pawn_moves(fixed_vector_t& legal_moves, const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player, const Bitboard& current_side_occupied_squares, const std::optional<Position>& en_passant_target_square, const Position& king_square = Position{-1, -1}, const Bitboard& pinned_pieces = {})
	{
		pawn_moves<moves_type>(legal_moves, pawns_bb, occupied_squares, active_player, current_side_occupied_squares, en_passant_target_square, [](const Move& move){ return true; }, king_square, pinned_pieces);
	}

	template <Moves_type moves_type, typename T>
	void knight_moves(fixed_vector_t& legal_moves, const Bitboard& knight_bb, const Bitboard& our_occupied_squares, const T& satisfies_check_requirements, const Bitboard& enemy_occupied_squares = {})
	{
		knight_bb.for_each_piece([&](const auto& origin_square)
		{
			Bitboard knight_moves = knight_mask(origin_square) & ~our_occupied_squares;
			if constexpr (moves_type == Moves_type::noisy)
				knight_moves &= enemy_occupied_squares;
			knight_moves.for_each_piece([&](const auto& destination_square)
			{
				if(const Move current_move = Move{origin_square, destination_square}; satisfies_check_requirements(current_move))
					legal_moves.push_back(current_move);
			});
		});
	}

	template <Moves_type moves_type>
	void knight_moves(fixed_vector_t& legal_moves, const Bitboard& knight_bb, const Bitboard& our_occupied_squares, const Bitboard& enemy_occupied_squares = {})
	{
		knight_moves<moves_type>(legal_moves, knight_bb, our_occupied_squares, [](const Move& move){ return true; }, enemy_occupied_squares);
	}

	template <Moves_type moves_type>
	void king_moves(fixed_vector_t& legal_moves, const Position& original_square, const Bitboard& occupied_squares, const Bitboard& our_occupied_squares, const bool is_white, const Castling_rights_map<bool>& castling_rights, const Bitboard& enemy_attack_map)
	{
		bool in_check = !is_free(original_square, enemy_attack_map);
		if(!in_check)
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
		if constexpr (moves_type == Moves_type::noisy)
		{
			if(!in_check)
				return;
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

	template <Moves_type moves_type, typename T>
	void rook_moves(fixed_vector_t& legal_moves, const Bitboard& rook_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const T& satisfies_check_requirements, const Bitboard& pinned_pieces = {}, const Bitboard& enemy_occupied_squares = {})
	{
		rook_bb.for_each_piece([&](const Position& original_square)
		{
			Bitboard rook_moves = rook_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares;
			if constexpr (moves_type == Moves_type::noisy)
				rook_moves &= enemy_occupied_squares;
			rook_moves.for_each_piece([&](const auto& destination_square)
			{
				const bool move_stays_pinned = (original_square.rank_ == king_square.rank_ && destination_square.rank_ == king_square.rank_) || (original_square.file_ == king_square.file_ && destination_square.file_ == king_square.file_);
				const Move current_move = Move{original_square, destination_square};
				if((!pinned_pieces.is_occupied(original_square) || move_stays_pinned) && satisfies_check_requirements(current_move))
					legal_moves.push_back(current_move);
			});
		});
	}

	template <Moves_type moves_type>
	void rook_moves(fixed_vector_t& legal_moves, const Bitboard& rook_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const Bitboard& pinned_pieces = {}, const Bitboard& enemy_occupied_squares = {})
	{
		rook_moves<moves_type>(legal_moves, rook_bb, occupied_squares, current_sides_occupied_squares, king_square, [](const Move& move){ return true; }, pinned_pieces, enemy_occupied_squares);
	}

	template <Moves_type moves_type, typename T>
	void bishop_moves(fixed_vector_t& legal_moves, const Bitboard& bishop_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const T& satisfies_check_requirements, const Bitboard& pinned_pieces = {}, const Bitboard& enemy_occupied_squares = {})
	{
		bishop_bb.for_each_piece([&](const Position& original_square)
		{
			Bitboard bishop_moves = bishop_legal_moves_bb(original_square, occupied_squares) & ~current_sides_occupied_squares;
			if constexpr (moves_type == Moves_type::noisy)
				bishop_moves &= enemy_occupied_squares;
			bishop_moves.for_each_piece([&](const auto& destination_square)
			{
				const bool move_stays_pinned = (original_square.diagonal_index() == king_square.diagonal_index() && destination_square.diagonal_index() == king_square.diagonal_index()) || (original_square.antidiagonal_index() == king_square.antidiagonal_index() && destination_square.antidiagonal_index() == king_square.antidiagonal_index());
				const Move current_move = Move{original_square, destination_square};
				if((!pinned_pieces.is_occupied(original_square) || move_stays_pinned) && satisfies_check_requirements(current_move))
				{
					if constexpr (moves_type == Moves_type::noisy)
					{
						if(!is_free(destination_square, enemy_occupied_squares))
							legal_moves.push_back(current_move);
					}
					else if constexpr (moves_type == Moves_type::legal)
						legal_moves.push_back(current_move);
				}
			});
		});
	}

	template <Moves_type moves_type>
	void bishop_moves(fixed_vector_t& legal_moves, const Bitboard& bishop_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const Bitboard& pinned_pieces = {}, const Bitboard& enemy_occupied_squares = {})
	{
		bishop_moves<moves_type>(legal_moves, bishop_bb, occupied_squares, current_sides_occupied_squares, king_square, [](const Move& move){ return true; }, pinned_pieces, enemy_occupied_squares);
	}

	template <Moves_type moves_type, typename T>
	void queen_moves(fixed_vector_t& legal_moves, const Bitboard& queen_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const T& satisfies_check_requirements, const Bitboard& pinned_pieces = {}, const Bitboard& enemy_occupied_squares = {})
	{
		bishop_moves<moves_type>(legal_moves, queen_bb, occupied_squares, current_sides_occupied_squares, king_square, satisfies_check_requirements, pinned_pieces, enemy_occupied_squares);
		rook_moves<moves_type>(legal_moves, queen_bb, occupied_squares, current_sides_occupied_squares, king_square, satisfies_check_requirements, pinned_pieces, enemy_occupied_squares);
	}

	template <Moves_type moves_type>
	void queen_moves(fixed_vector_t& legal_moves, const Bitboard& queen_bb, const Bitboard& occupied_squares, const Bitboard& current_sides_occupied_squares, const Position& king_square, const Bitboard& pinned_pieces = {}, const Bitboard& enemy_occupied_squares = {})
	{
		queen_moves<moves_type>(legal_moves, queen_bb, occupied_squares, current_sides_occupied_squares, king_square, [](const Move& move){ return true; }, pinned_pieces, enemy_occupied_squares);
	}

	[[nodiscard]] const Bitboard generate_pinned_pieces(const State& state, const Position& king_square) noexcept
	{
		Bitboard pinned_pieces{0ULL};
		const auto& side = state.sides[state.side_to_move];
		const auto& enemy_side = state.sides[other_side(state.side_to_move)];
		auto generate_pins_from=[&](const Bitboard& sliding_pieces, const auto& legal_moves)
		{
			Bitboard pinning_rays{0ULL};
			sliding_pieces.for_each_piece([&](const Position& position)
			{
				const Bitboard pinning_ray{legal_moves(position, state.occupied_squares())};
				pinning_rays|=pinning_ray;
			});
			return legal_moves(king_square, state.occupied_squares()) & pinning_rays & side.occupied_squares();
		};
		const Bitboard enemy_bishop_likes=enemy_side.pieces[Piece::bishop] | enemy_side.pieces[Piece::queen];
		if(const Bitboard attacking_bishop_likes=bishop_legal_moves_bb(king_square, enemy_bishop_likes) & enemy_bishop_likes; (attacking_bishop_likes)>0)
			pinned_pieces|=generate_pins_from(attacking_bishop_likes, bishop_legal_moves_bb);

		const Bitboard enemy_rook_likes=enemy_side.pieces[Piece::rook] | enemy_side.pieces[Piece::queen];
		if(const Bitboard attacking_rook_likes=rook_legal_moves_bb(king_square, enemy_rook_likes) & enemy_rook_likes; (attacking_rook_likes) > 0)
			pinned_pieces|=generate_pins_from(attacking_rook_likes, rook_legal_moves_bb);
		return pinned_pieces;
	}
}

namespace engine
{
	template <Moves_type moves_type>
	Fixed_capacity_vector<Move, max_legal_moves> generate_moves(const State& state) noexcept
	{
		Fixed_capacity_vector<Move, max_legal_moves> legal_moves{};
		const Side_position our_side = state.sides[state.side_to_move];
		const auto our_occupied_squares = our_side.occupied_squares(),
		enemy_occupied_squares = state.sides[other_side(state.side_to_move)].occupied_squares(),
		occupied_squares = state.occupied_squares();
		const Position king_square = our_side.pieces[Piece::king].lsb_square();
		const bool in_check = state.in_check();
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
		const Side_position& enemy_side = state.sides[other_side(state.side_to_move)];
		find_checking_pieces(enemy_side.pieces[Piece::rook] | enemy_side.pieces[Piece::queen], rook_moves_);
		find_checking_pieces(enemy_side.pieces[Piece::bishop] | enemy_side.pieces[Piece::queen], bishop_moves_);
		const Bitboard attacking_knights = knight_mask(king_square) & enemy_side.pieces[Piece::knight];
		const auto& pieces = our_side.pieces;
		const Bitboard& our_king_bb = pieces[Piece::king];
		const Bitboard left_attacking_pawn = state.side_to_move == Side::white? (our_king_bb & ~file_h) << (board_size+1) : (our_king_bb & ~file_h) >> (board_size-1);
		const Bitboard right_attacking_pawn = state.side_to_move == Side::white? (our_king_bb & ~file_a) << (board_size-1) : (our_king_bb & ~file_a) >> (board_size+1);
		const Bitboard attacking_pawns = enemy_side.pieces[Piece::pawn] & (left_attacking_pawn | right_attacking_pawn);
		const auto attacking_knights_popcount = attacking_knights.popcount(),
		attacking_pawns_popcount = attacking_pawns.popcount();
		king_moves<moves_type>(legal_moves, king_square, occupied_squares, our_occupied_squares | state.enemy_attack_map, state.side_to_move == Side::white? true : false, our_side.castling_rights, state.enemy_attack_map);
		const bool single_checking_blockable_piece = checking_pieces.popcount() == 1 && attacking_knights_popcount == 0 && attacking_pawns_popcount == 0,
		single_checking_unblockable_piece = checking_pieces.popcount() < 1 && (attacking_knights_popcount == 1 || attacking_pawns_popcount == 1),
		in_double_check = !single_checking_blockable_piece && !single_checking_unblockable_piece && in_check;
		if(in_double_check) [[unlikely]]
			return legal_moves;
		checking_pieces |= attacking_knights | attacking_pawns;
		const auto satisfies_check_requirements = [&, checking_piece_square=checking_pieces.lsb_square()](const Move& move)
		{
			if(single_checking_blockable_piece)
			{
				const Position destination_square = move.destination_square();
				if(king_square.rank_ == checking_piece_square.rank_)
					return destination_square.rank_ == king_square.rank_ && destination_square.file_ >= std::min(king_square.file_, checking_piece_square.file_) && destination_square.file_ <= std::max(king_square.file_, checking_piece_square.file_);
				if(king_square.file_ == checking_piece_square.file_)
					return destination_square.file_ == checking_piece_square.file_ && destination_square.rank_ >= std::min(king_square.rank_, checking_piece_square.rank_) && destination_square.rank_ <= std::max(king_square.rank_, checking_piece_square.rank_);
				if(king_square.diagonal_index() == checking_piece_square.diagonal_index())
					return destination_square.diagonal_index() == king_square.diagonal_index() && destination_square.rank_ >= std::min(king_square.rank_, checking_piece_square.rank_) && destination_square.rank_ <= std::max(king_square.rank_, checking_piece_square.rank_);
				if(king_square.antidiagonal_index() == checking_piece_square.antidiagonal_index())
					return destination_square.antidiagonal_index() == king_square.antidiagonal_index() && destination_square.rank_ >= std::min(king_square.rank_, checking_piece_square.rank_) && destination_square.rank_ <= std::max(king_square.rank_, checking_piece_square.rank_);
				std::unreachable();
			}
			else if(single_checking_unblockable_piece)
				return move.destination_square() == checking_piece_square;
			else
				return true; // !in_check
		};
		const Bitboard pinned_pieces = generate_pinned_pieces(state, king_square);
		pawn_moves<moves_type>(legal_moves, pieces[Piece::pawn], occupied_squares, state.side_to_move, our_occupied_squares, state.en_passant_target_square, satisfies_check_requirements, king_square, pinned_pieces);
		knight_moves<moves_type>(legal_moves, pieces[Piece::knight] & ~pinned_pieces, our_occupied_squares, satisfies_check_requirements, enemy_occupied_squares);

		bishop_moves<moves_type>(legal_moves, pieces[Piece::bishop], occupied_squares, our_occupied_squares, king_square,satisfies_check_requirements, pinned_pieces, enemy_occupied_squares);
		rook_moves<moves_type>(legal_moves, pieces[Piece::rook], occupied_squares, our_occupied_squares, king_square, satisfies_check_requirements,  pinned_pieces, enemy_occupied_squares);
		queen_moves<moves_type>(legal_moves, pieces[Piece::queen], occupied_squares, our_occupied_squares, king_square, satisfies_check_requirements, pinned_pieces, enemy_occupied_squares);
		return legal_moves;
	}

	template Fixed_capacity_vector<Move, max_legal_moves> generate_moves<Moves_type::legal>(const State& state) noexcept;
	template Fixed_capacity_vector<Move, max_legal_moves> generate_moves<Moves_type::noisy>(const State& state) noexcept;

	Bitboard generate_attack_map(const State& state) noexcept
	{
		Bitboard attack_map{0ULL};
		const auto& pieces=state.sides[state.side_to_move].pieces;
		const auto& pawn_bb=pieces[Piece::pawn];
		const bool is_white=state.side_to_move == Side::white;
		auto occupied_squares = state.occupied_squares();
		occupied_squares.remove_piece(state.sides[other_side(state.side_to_move)].pieces[Piece::king].lsb_square());
		attack_map|=is_white? ((pawn_bb & ~file_a) << (board_size-1)) : ((pawn_bb & ~file_a) >> (board_size+1));
		attack_map|=is_white? ((pawn_bb & ~file_h) << (board_size+1)) : ((pawn_bb & ~file_h) >> (board_size-1));
		attack_map|=king_mask(pieces[Piece::king].lsb_square());
		pieces[Piece::knight].for_each_piece([&](const Position& position){ attack_map|=knight_mask(position); });
		(pieces[Piece::bishop] | pieces[Piece::queen]).for_each_piece([&](const Position& position){ attack_map|=bishop_legal_moves_bb(position, occupied_squares); });
		(pieces[Piece::rook] | pieces[Piece::queen]).for_each_piece([&](const Position& position){ attack_map|=rook_legal_moves_bb(position, occupied_squares); });
		return attack_map;
	}
}
