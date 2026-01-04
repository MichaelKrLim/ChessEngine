#include "move_unmove.h"

#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move_generator.h"
#include "State.h"
#include "Transposition_table.h"

namespace
{
	using namespace engine;

	void modify_at(const Position& square, const Piece piece_to_modify, void (Bitboard::*modifier)(const Position&), const Side current_side, State& state) noexcept
	{
		std::invoke(modifier, state.sides[current_side].pieces[piece_to_modify], square);
		zobrist::invert_piece_at(state.zobrist_hash, square, piece_to_modify, state.side_to_move);
	}

	void modify_features(const Position& square, const Piece piece_to_modify, const Side current_side, Side_map<Fixed_capacity_vector<std::uint16_t,4>>& modified_features, const Side_map<Position>& king_squares) noexcept
	{
		if(piece_to_modify!=Piece::king)
		{
			for(const auto side : all_sides)
				modified_features[side].push_back(Neural_network::compute_feature_index(piece_to_modify, square, king_squares[side], current_side, side));
		}
	}

	void modify_at(const Position& square, const Piece piece_to_modify, void (Bitboard::*modifier)(const Position&), const Side current_side, State& state, Side_map<Fixed_capacity_vector<std::uint16_t,4>>& modified_features, const Side_map<Position>& king_squares) noexcept
	{
		modify_at(square, piece_to_modify, modifier, current_side, state);
		modify_features(square, piece_to_modify, current_side, modified_features, king_squares);
	}

	void make(State& state, const Move& move, const Piece& piece_type, const auto& add_at, const auto& remove_at) noexcept
	{
		Side_position& side{state.sides[state.side_to_move]};
		const Side enemy_side{other_side(state.side_to_move)};
		auto& opposite_side{state.sides[enemy_side]};
		const auto destination_square{move.destination_square()};
		const auto from_square{move.from_square()};
		const auto pawn_direction{state.side_to_move == Side::white? 1 : -1};
		const auto back_rank{state.side_to_move == Side::white? 0 : 7};
		const auto white_old_castling_rights{state.sides[Side::white].castling_rights};
		const auto black_old_castling_rights{state.sides[Side::black].castling_rights};
		const auto old_zobrist_hash{state.zobrist_hash};

		const std::optional<Piece> piece_to_capture{state.piece_at(destination_square, enemy_side)};
		const Side_map<Position> king_squares=[&]()
		{
			Side_map<Position> king_squares;
			for(const auto side : all_sides)
				king_squares[side]=state.sides[side].pieces[Piece::king].lsb_square();
			return king_squares;
		}();

		const auto handle_capture = [&, enemy_back_rank=state.side_to_move == Side::white? 7 : 0, enemy_side]()
		{
			remove_at(destination_square, piece_to_capture.value(), enemy_side, king_squares);
			if(piece_to_capture==Piece::rook)
			{
				if(destination_square==Position{enemy_back_rank, 0} && opposite_side.castling_rights[Castling_rights::queenside])
				{
					opposite_side.castling_rights[Castling_rights::queenside]=false;
					zobrist::invert_castling_right(state.zobrist_hash, other_side(state.side_to_move), Castling_rights::queenside);
				}
				else if(destination_square == Position{enemy_back_rank, 7} && opposite_side.castling_rights[Castling_rights::kingside])
				{
					opposite_side.castling_rights[Castling_rights::kingside]=false;
					zobrist::invert_castling_right(state.zobrist_hash, other_side(state.side_to_move), Castling_rights::kingside);
				}
			}
		};

		const auto move_and_hash = [&](const Position& from_square, const Position& destination_square, const Piece& piece_type_to_move)
		{
			remove_at(from_square, piece_type_to_move, state.side_to_move, king_squares);
			add_at(destination_square, piece_type_to_move, state.side_to_move, king_squares);
		};

		const auto handle_castling = [&]()
		{
			bool castled_kingside=destination_square.file_==6;
			Position rook_destination_square, rook_origin_square;
			if(castled_kingside)
			{
				rook_origin_square=Position{back_rank, 7};
				rook_destination_square=Position{back_rank, destination_square.file_-1};
			}
			else
			{
				rook_origin_square=Position{back_rank, 0};
				rook_destination_square=Position{back_rank, destination_square.file_+1};
			}
			for(const auto& castling_right : all_castling_rights)
			{
				if(side.castling_rights[castling_right])
				{
					side.castling_rights[castling_right]=false;
					zobrist::invert_castling_right(state.zobrist_hash, state.side_to_move, castling_right);
				}
			}
			move_and_hash(from_square, destination_square, Piece::king);
			move_and_hash(rook_origin_square, rook_destination_square, Piece::rook);
		};

		if(piece_to_capture)
			handle_capture();
		const bool is_castling = piece_type == Piece::king && std::abs(destination_square.file_-from_square.file_) > 1,
				   is_en_passant = state.en_passant_target_square && piece_type == Piece::pawn && destination_square == state.en_passant_target_square.value();
		const auto old_en_passant_target_square = state.en_passant_target_square;
		if(state.en_passant_target_square)
		{
			zobrist::invert_en_passant_square(state.zobrist_hash, state.en_passant_target_square.value());
			state.en_passant_target_square = std::nullopt;
		}
		if(move.is_promotion())
		{
			const auto promotion_piece{move.promotion_piece()};
			remove_at(from_square, Piece::pawn, state.side_to_move, king_squares);
			add_at(destination_square, promotion_piece, state.side_to_move, king_squares);
		}
		else if(is_castling)
			handle_castling();
		else
		{
			const bool is_double_pawn_move = piece_type == Piece::pawn && destination_square.rank_ == from_square.rank_ + 2*pawn_direction;
			if(is_double_pawn_move)
			{
				state.en_passant_target_square = Position{from_square.rank_+pawn_direction, from_square.file_};
				zobrist::invert_en_passant_square(state.zobrist_hash, state.en_passant_target_square.value());
			}
			if(is_en_passant)
			{
				const Position capture_square{destination_square.rank_-pawn_direction, destination_square.file_};
				remove_at(capture_square, Piece::pawn, enemy_side, king_squares);
			}
			move_and_hash(from_square, destination_square, piece_type);
		}

		state.history.emplace
		(
			move,
			piece_type,
			piece_to_capture,
			state.enemy_attack_map,
			old_en_passant_target_square,
			is_en_passant,
			white_old_castling_rights,
			black_old_castling_rights,
			old_zobrist_hash, 
			state.half_move_clock
		);

		if(piece_type == Piece::rook || piece_type == Piece::king)
		{
			if((from_square.file_ == 0 || piece_type == Piece::king) && side.castling_rights[Castling_rights::queenside])
			{
				side.castling_rights[Castling_rights::queenside] = false;
				zobrist::invert_castling_right(state.zobrist_hash, state.side_to_move, Castling_rights::queenside);
			}
			if((from_square.file_ == 7 || piece_type == Piece::king) && side.castling_rights[Castling_rights::kingside])
			{
				side.castling_rights[Castling_rights::kingside] = false;
				zobrist::invert_castling_right(state.zobrist_hash, state.side_to_move, Castling_rights::kingside);
			}
		}

		if(piece_type == Piece::pawn || piece_to_capture)
			state.half_move_clock = 0;
		else
			++state.half_move_clock;
		if(state.side_to_move == Side::black)
			++state.full_move_clock;
		state.enemy_attack_map = generate_attack_map(state);
		zobrist::invert_side_to_move(state.zobrist_hash);
		state.side_to_move = enemy_side;
		state.repetition_history.push_back(state.zobrist_hash);
	}

	void unmove(State& state, const State::Delta& history_data, const auto& add_at, const auto& remove_at) noexcept
	{
		const bool was_whites_move = state.side_to_move == Side::black;
		const auto last_moved_side = was_whites_move? Side::white : Side::black;
		const auto promotion_rank = was_whites_move? 7 : 0;	
		const Side_map<Position> king_squares=[&]()
		{
			Side_map<Position> king_squares;
			for(const auto side : all_sides)
				king_squares[side]=state.sides[side].pieces[Piece::king].lsb_square();
			return king_squares;
		}();
		state.half_move_clock=history_data.half_move_clock;
		if(!was_whites_move)
			--state.full_move_clock;
		const Position& previous_move_destination = history_data.move.destination_square(), previous_move_origin = history_data.move.from_square();
		if(history_data.piece == Piece::pawn && previous_move_destination.rank_ == promotion_rank)
		{
			remove_at(previous_move_destination, history_data.move.promotion_piece(), last_moved_side, king_squares);
			add_at(previous_move_origin, Piece::pawn, last_moved_side, king_squares);
		}
		else if(history_data.piece == Piece::king && std::abs(previous_move_destination.file_ - previous_move_origin.file_) > 1)
		{
			const std::uint8_t rank = last_moved_side == Side::white? 0 : 7;
			Position rook_destination_square, rook_origin_square;
			bool castled_kingside = previous_move_destination.file_ == 6;
			if(castled_kingside)
			{
				rook_origin_square = Position{rank, 7};
				rook_destination_square = Position{rank, 5};
			}
			else
			{
				rook_origin_square = Position{rank, 0};
				rook_destination_square = Position{rank, 3};
			}
			add_at(previous_move_origin, Piece::king, last_moved_side, king_squares);
			remove_at(previous_move_destination, Piece::king, last_moved_side, king_squares);

			add_at(rook_origin_square, Piece::rook, last_moved_side, king_squares);
			remove_at(rook_destination_square, Piece::rook, last_moved_side, king_squares);
		}
		else
		{
			if(history_data.was_en_passant)
			{
				const auto direction = was_whites_move? 1:-1;
				const Position pawn_to_return{previous_move_destination.rank_-direction, previous_move_destination.file_};
				add_at(pawn_to_return, Piece::pawn, state.side_to_move, king_squares);
			}
			remove_at(previous_move_destination, history_data.piece, last_moved_side, king_squares);
			add_at(previous_move_origin, history_data.piece, last_moved_side, king_squares);
		}

		if(history_data.captured_piece)
			add_at(previous_move_destination, *history_data.captured_piece, state.side_to_move, king_squares);

		state.sides[Side::white].castling_rights=history_data.white_castling_rights;
		state.sides[Side::black].castling_rights=history_data.black_castling_rights;


		state.side_to_move = last_moved_side;
		state.enemy_attack_map = history_data.enemy_attack_map;
		state.en_passant_target_square = history_data.en_passant_target_square;

		state.repetition_history.pop_back();
		state.zobrist_hash = history_data.previous_zobrist_hash;
	}
}

namespace engine
{
	void make(State& state, Accumulator& accumulator, const Move& move, const Neural_network& neural_network) noexcept
	{
		Side_map<Fixed_capacity_vector<std::uint16_t,4>> added_features;
		Side_map<Fixed_capacity_vector<std::uint16_t,4>> removed_features;

		const auto add_at = [&](const Position& square_to_add_at, const Piece piece_to_add, const Side side, Side_map<Position> king_squares)
		{
			modify_at(square_to_add_at, piece_to_add, &Bitboard::add_piece, side, state, added_features, king_squares);
		};
		const auto remove_at = [&](const Position& square_to_remove_at, const Piece piece_to_remove, const Side side, Side_map<Position> king_squares)
		{
			modify_at(square_to_remove_at, piece_to_remove, &Bitboard::remove_piece, side, state, removed_features, king_squares);
		};

		const Piece piece_type{*state.piece_at(move.from_square(), state.side_to_move)};
		::make(state, move, piece_type, add_at, remove_at);

		change_accumulator(state, removed_features, added_features, other_side(state.side_to_move), piece_type, neural_network, accumulator);
	}

	void unmove(State& state, Accumulator& accumulator, const Neural_network& neural_network) noexcept
	{
		Side_map<Fixed_capacity_vector<std::uint16_t,4>> added_features;
		Side_map<Fixed_capacity_vector<std::uint16_t,4>> removed_features;

		const auto history_data{std::move(state.history.top())};
		state.history.pop();

		const auto add_at = [&](const Position& square_to_add_at, const Piece piece_to_add, const Side side, Side_map<Position> king_squares)
		{
			state.sides[side].pieces[piece_to_add].add_piece(square_to_add_at);
			modify_features(square_to_add_at, piece_to_add, side, added_features, king_squares);
		};
		const auto remove_at = [&](const Position& square_to_remove_at, const Piece piece_to_remove, const Side side, Side_map<Position> king_squares)
		{
			state.sides[side].pieces[piece_to_remove].remove_piece(square_to_remove_at);
			modify_features(square_to_remove_at, piece_to_remove, side, removed_features, king_squares);
		};

		::unmove(state, history_data, add_at, remove_at);

		change_accumulator(state, removed_features, added_features, other_side(state.side_to_move), history_data.piece, neural_network, accumulator);
	}

	void make(State& state, const Move& move) noexcept
	{
		const Piece piece_type{*state.piece_at(move.from_square(), state.side_to_move)};
		const auto add_at = [&state](const Position& square_to_add_at, const Piece piece_to_add, const Side side, const Side_map<Position>&)
		{
			modify_at(square_to_add_at, piece_to_add, &Bitboard::add_piece, side, state);
		};
		const auto remove_at = [&state](const Position& square_to_remove_at, const Piece piece_to_remove, const Side side, const Side_map<Position>&)
		{
			modify_at(square_to_remove_at, piece_to_remove, &Bitboard::remove_piece, side, state);
		};

		::make(state, move, piece_type, add_at, remove_at);
	}

	void unmove(State& state) noexcept
	{
		const auto history_data{std::move(state.history.top())};
		state.history.pop();

		const auto add_at = [&](const Position& square_to_add_at, const Piece piece_to_add, const Side side, Side_map<Position> king_squares)
		{
			state.sides[side].pieces[piece_to_add].add_piece(square_to_add_at);
		};
		const auto remove_at = [&](const Position& square_to_remove_at, const Piece piece_to_remove, const Side side, Side_map<Position> king_squares)
		{
			state.sides[side].pieces[piece_to_remove].remove_piece(square_to_remove_at);
		};

		::unmove(state, history_data, add_at, remove_at);
	}
}
