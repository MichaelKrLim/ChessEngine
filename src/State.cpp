#include "Chess_data.h"
#include "Move_generator.h"
#include "Move.h"
#include "nnue/Neural_network.h"
#include "State.h"
#include "Transposition_table.h"

#include <algorithm>
#include <ranges>
#include <regex>
#include <sstream>

using namespace engine;

void State::validate_fen(const std::array<std::string, 6>& partitioned_fen) const
{
	const auto [fen_piece_data, fen_active_color, fen_castling_availiability, fen_en_passant_target_square, fen_halfmove_clock, fen_fullmove_clock] = partitioned_fen;
	auto ranks = std::ranges::views::split(fen_piece_data, '/');
	if(std::ranges::distance(ranks) != 8)
		throw std::invalid_argument("invalid piece data");
	for(auto&& rank : ranks)
	{
		const bool double_empty_squares = std::regex_match(std::string{rank.begin(), rank.end()}, std::regex{"\\d{2}"});
		const bool invalid_letters = std::ranges::any_of(rank, [](const char& letter){ return !std::regex_match(std::string{letter}, std::regex{"[1-8]|[pkqbnrPKQBNR]"}); });
		const std::uint8_t number_of_squares = std::ranges::fold_left(rank, 0, [](int current_count, const char& letter){ return isdigit(letter)? current_count+letter-'0' : current_count+1; });
		if(invalid_letters || number_of_squares != 8 || double_empty_squares)
			throw std::invalid_argument("invalid piece data");
		if(fen_active_color != "w" && fen_active_color != "b")
			throw std::invalid_argument("invalid active color");
		if(!std::regex_match(fen_castling_availiability, std::regex{"^-$|^(KQ?k?q?|Qk?q?|kq?|q)$"}))
			throw std::invalid_argument("invalid castling rights");
		if(!std::regex_match(fen_en_passant_target_square, std::regex{"^(-|[a-h][36])$"}))
			throw std::invalid_argument("invalid en passant square");
		if(!std::ranges::all_of(fen_halfmove_clock, ::isdigit))
			throw std::invalid_argument("invalid halfmove clock");
		if(!std::ranges::all_of(fen_fullmove_clock, ::isdigit))
			throw std::invalid_argument("invalid halfmove clock");
	}
}

void State::parse_fen(const std::string_view fen) noexcept
{
	const auto to_piece = [](const char& to_convert)
	{
		constexpr auto to_piece = []()
		{
			std::array<Piece, 256> to_piece = {};
			to_piece['p'] = Piece::pawn;
			to_piece['n'] = Piece::knight;
			to_piece['b'] = Piece::bishop;
			to_piece['q'] = Piece::queen;
			to_piece['k'] = Piece::king;
			to_piece['r'] = Piece::rook;
			return to_piece;
		}();
		return to_piece[to_convert];
	};
	const auto to_shift = [](std::size_t board_index)
	{
		Position position(board_index);
		const std::size_t flipped = (board_size-1-position.rank_)*board_size+position.file_;
		return flipped;
	};
	const auto read_piece_data = [&](const std::string_view& fen_section)
	{
		std::size_t board_index{0};
		for(std::size_t i{0}; i<fen_section.size(); ++i)
		{
			const auto add_piece = [&](const Side& side)
			{
				const Piece piece_type{to_piece(std::tolower(fen_section[i]))};
				const Bitboard mask{Bitboard{1ULL} << to_shift(board_index)};
				sides[side].pieces[piece_type] |= mask;
				evaluation+=chess_data::piece_values[side][piece_type]+chess_data::weightmaps[side][piece_type][board_index];
			};
			if(fen_section[i] == '/')
				continue;
			else if(std::isdigit(fen_section[i]))
				board_index+=fen_section[i]-'0';
			else if(std::isupper(fen_section[i]))
			{
				add_piece(Side::white);
				++board_index;
			}
			else
			{
				add_piece(Side::black);
				++board_index;
			}
		}
	};

	std::istringstream iss{std::string{fen}};
	std::array<std::string, 6> partitioned_fen;
	for(auto& partition : partitioned_fen)
		std::getline(iss, partition, ' ');
	for(bool invalid_input{true}; invalid_input;)
	{
		try
		{
			validate_fen(partitioned_fen);
			invalid_input = false;
		}
		catch(const std::invalid_argument& exception)
		{
			std::cout << "Invalid fen string: " << exception.what() << "\n";
			std::string new_fen;
			std::getline(std::cin, new_fen);
			std::istringstream iss{std::string{new_fen}};
			for(auto& partition : partitioned_fen)
				std::getline(iss, partition, ' ');
		}
	}
	const auto [fen_piece_data, fen_active_color, fen_castling_availiability, fen_en_passant_target_square, fen_halfmove_clock, fen_fullmove_clock] = partitioned_fen;
	read_piece_data(fen_piece_data);
	side_to_move = fen_active_color[0] == 'w'? Side::white : Side::black;
	if(fen_castling_availiability[0] != '-')
	{
		for(const char& castling_right : fen_castling_availiability)
		{
			Side side_index;
			Castling_rights castling_right_index;
			if(std::isupper(castling_right))
				side_index = Side::white;
			else
				side_index = Side::black;
			if(std::tolower(castling_right) == 'k')
				castling_right_index = Castling_rights::kingside;
			else
				castling_right_index = Castling_rights::queenside;
			sides[side_index].castling_rights[castling_right_index] = true;
		}
	}
	if(fen_en_passant_target_square[0] != '-')
		en_passant_target_square = algebraic_to_position(fen_en_passant_target_square);
	half_move_clock = std::stoi(fen_halfmove_clock);
	full_move_clock = std::stoi(fen_fullmove_clock);
}

State::State(const std::string_view fen)
{
	repetition_history.reserve(max_depth);
	parse_fen(fen);
	side_to_move = other_side(side_to_move);
	enemy_attack_map = generate_attack_map(*this);
	side_to_move = other_side(side_to_move);
	zobrist_hash = zobrist::hash(*this);
	repetition_history.push_back(zobrist_hash);
	for(const auto side : all_sides)
		neural_network.refresh_accumulator(to_halfKP_features(side), side);
}

std::optional<Piece> State::piece_at(const Position& position, const Side& side) const noexcept
{
	std::optional<Piece> found_piece{std::nullopt};
	for(const auto& piece : all_pieces)
	{
		if(!is_free(position, sides[side].pieces[piece]))
		{
			found_piece = piece;
			break;
		}
	}
	return found_piece;
}

void State::update_accumulator(const auto& removed_features, const auto& added_features, const Side moved_side, const Piece moved_piece) noexcept
{
	const Side enemy_side{other_side(moved_side)};
	neural_network.update_accumulator(removed_features[enemy_side], added_features[enemy_side], enemy_side);
	if(moved_piece==Piece::king)
		neural_network.refresh_accumulator(to_halfKP_features(moved_side), moved_side);
	else
		neural_network.update_accumulator(removed_features[moved_side], added_features[moved_side], moved_side);
}

void State::make(const Move& move) noexcept
{
	Side_position& side{sides[side_to_move]};
	const Side enemy_side{other_side(side_to_move)};
	auto& opposite_side{sides[enemy_side]};
	const auto destination_square{move.destination_square()};
	const auto from_square{move.from_square()};
	const auto pawn_direction{side_to_move == Side::white? 1 : -1};
	const auto back_rank{side_to_move == Side::white? 0 : 7};
	const auto white_old_castling_rights{sides[Side::white].castling_rights};
	const auto black_old_castling_rights{sides[Side::black].castling_rights};
	const auto old_zobrist_hash{zobrist_hash};
	const auto old_evaluation{evaluation};
	const Piece piece_type{*piece_at(from_square, side_to_move)};
	const std::optional<Piece> piece_to_capture{piece_at(destination_square, enemy_side)};
	const Side_map<Position> king_squares=[&]()
	{
		Side_map<Position> king_squares;
		for(const auto side : all_sides)
			king_squares[side]=sides[side].pieces[Piece::king].lsb_square();
		return king_squares;
	}();
	Side_map<Fixed_capacity_vector<std::uint16_t,4>> added_features;
	Side_map<Fixed_capacity_vector<std::uint16_t,4>> removed_features;

	const auto handle_capture = [&, this, enemy_back_rank=side_to_move == Side::white? 7 : 0, enemy_side]()
	{
		opposite_side.pieces[piece_to_capture.value()].remove_piece(destination_square);
		for(const auto side : all_sides)
			removed_features[side].push_back(Neural_network::compute_feature_index(piece_to_capture.value(), destination_square, king_squares[side], enemy_side, side));
		evaluation-=chess_data::piece_values[other_side(side_to_move)][piece_to_capture.value()]+chess_data::weightmaps[enemy_side][piece_to_capture.value()][to_index(destination_square)];
		zobrist::invert_piece_at(zobrist_hash, destination_square, piece_to_capture.value(), other_side(side_to_move));
		if(piece_to_capture==Piece::rook)
		{
			if(destination_square==Position{enemy_back_rank, 0} && opposite_side.castling_rights[Castling_rights::queenside])
			{
				opposite_side.castling_rights[Castling_rights::queenside]=false;
				zobrist::invert_castling_right(zobrist_hash, other_side(side_to_move), Castling_rights::queenside);
			}
			else if(destination_square == Position{enemy_back_rank, 7} && opposite_side.castling_rights[Castling_rights::kingside])
			{
				opposite_side.castling_rights[Castling_rights::kingside]=false;
				zobrist::invert_castling_right(zobrist_hash, other_side(side_to_move), Castling_rights::kingside);
			}
		}
	};

	const auto move_and_hash = [&, this](const Position& from_square, const Position& destination_square, const Piece& piece_type_to_move)
	{
		sides[side_to_move].pieces[piece_type_to_move].move_piece(from_square, destination_square);
		if(piece_type_to_move!=Piece::king)
		{
			for(const auto side : all_sides)
			{
				removed_features[side].push_back(Neural_network::compute_feature_index(piece_type_to_move, from_square, king_squares[side], side_to_move, side));
				added_features[side].push_back(Neural_network::compute_feature_index(piece_type_to_move, destination_square, king_squares[side], side_to_move, side));
			}
		}
		zobrist::invert_piece_at(zobrist_hash, from_square, piece_type_to_move, side_to_move);
		zobrist::invert_piece_at(zobrist_hash, destination_square, piece_type_to_move, side_to_move);
		const auto& weightmap=chess_data::weightmaps[side_to_move][piece_type_to_move];
		const auto delta_positional_value{weightmap[to_index(destination_square)]-weightmap[to_index(from_square)]};
		evaluation+=delta_positional_value;
	};

	const auto handle_castling = [this, &destination_square, &back_rank, &side, &from_square, &move_and_hash]()
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
				zobrist::invert_castling_right(zobrist_hash, side_to_move, castling_right);
			}
		}
		move_and_hash(from_square, destination_square, Piece::king);
		move_and_hash(rook_origin_square, rook_destination_square, Piece::rook);
	};

	if(piece_to_capture)
		handle_capture();
	const bool is_castling = piece_type == Piece::king && std::abs(destination_square.file_-from_square.file_) > 1,
			   is_en_passant = en_passant_target_square && piece_type == Piece::pawn && destination_square == en_passant_target_square.value();
	const auto old_en_passant_target_square = en_passant_target_square;
	if(en_passant_target_square)
	{
		zobrist::invert_en_passant_square(zobrist_hash, en_passant_target_square.value());
		en_passant_target_square = std::nullopt;
	}
	if(move.is_promotion())
	{
		const auto promotion_piece = move.promotion_piece();
		side.pieces[Piece::pawn].remove_piece(from_square);
		const auto& weightmap=chess_data::weightmaps[side_to_move];
		const auto& side_piece_values=chess_data::piece_values[side_to_move];
		evaluation-=side_piece_values[Piece::pawn]+weightmap[Piece::pawn][to_index(from_square)];
		zobrist::invert_piece_at(zobrist_hash, from_square, Piece::pawn, side_to_move);
		side.pieces[promotion_piece].add_piece(destination_square);
		evaluation+=side_piece_values[promotion_piece]+weightmap[promotion_piece][to_index(destination_square)];
		zobrist::invert_piece_at(zobrist_hash, destination_square, promotion_piece, side_to_move);
		for(const auto side : all_sides)
		{
			added_features[side].push_back(Neural_network::compute_feature_index(promotion_piece, destination_square, king_squares[side], side_to_move, side));
			removed_features[side].push_back(Neural_network::compute_feature_index(Piece::pawn, from_square, king_squares[side], side_to_move, side));
		}
	}
	else if(is_castling)
		handle_castling();
	else
	{
		const bool is_double_pawn_move = piece_type == Piece::pawn && destination_square.rank_ == from_square.rank_ + 2*pawn_direction;
		if(is_double_pawn_move)
		{
			en_passant_target_square = Position{from_square.rank_+pawn_direction, from_square.file_};
			zobrist::invert_en_passant_square(zobrist_hash, en_passant_target_square.value());
		}
		if(is_en_passant)
		{
			const Position capture_square{destination_square.rank_-pawn_direction, destination_square.file_};
			opposite_side.pieces[Piece::pawn].remove_piece(capture_square);
			for(const auto side : all_sides)
				removed_features[side].push_back(Neural_network::compute_feature_index(Piece::pawn, capture_square, king_squares[side], enemy_side, side));
			evaluation-=chess_data::piece_values[other_side(side_to_move)][Piece::pawn]+chess_data::weightmaps[enemy_side][Piece::pawn][to_index(capture_square)];
			zobrist::invert_piece_at(zobrist_hash, capture_square, Piece::pawn, other_side(side_to_move));
		}
		move_and_hash(from_square, destination_square, piece_type);
	}

	history.emplace(State_delta
	{
		move,
		piece_type,
		piece_to_capture,
		enemy_attack_map,
		old_en_passant_target_square,
		is_en_passant,
		white_old_castling_rights,
		black_old_castling_rights,
		old_zobrist_hash, 
		half_move_clock,
		old_evaluation,
	});

	update_accumulator(removed_features, added_features, side_to_move, piece_type);

	if(piece_type == Piece::rook || piece_type == Piece::king)
	{
		if((from_square.file_ == 0 || piece_type == Piece::king) && side.castling_rights[Castling_rights::queenside])
		{
			side.castling_rights[Castling_rights::queenside] = false;
			zobrist::invert_castling_right(zobrist_hash, side_to_move, Castling_rights::queenside);
		}
		if((from_square.file_ == 7 || piece_type == Piece::king) && side.castling_rights[Castling_rights::kingside])
		{
			side.castling_rights[Castling_rights::kingside] = false;
			zobrist::invert_castling_right(zobrist_hash, side_to_move, Castling_rights::kingside);
		}
	}

	if(piece_type == Piece::pawn || piece_to_capture)
		half_move_clock = 0;
	else
		++half_move_clock;
	if(side_to_move == Side::black)
		++full_move_clock;
	enemy_attack_map = generate_attack_map(*this);
	zobrist::invert_side_to_move(zobrist_hash);
	side_to_move = enemy_side;
	repetition_history.push_back(zobrist_hash);
}

void State::unmove() noexcept
{
	assert(history.size() > 0 && "Tried to undo noexistent move");
	const bool was_whites_move = side_to_move == Side::black;
	const auto last_moved_side = was_whites_move? Side::white : Side::black;
	const auto promotion_rank = was_whites_move? 7 : 0;	
	const auto history_data = std::move(history.top());
	history.pop();
	const Side_map<Position> king_squares=[&]()
	{
		Side_map<Position> king_squares;
		for(const auto side : all_sides)
			king_squares[side]=sides[side].pieces[Piece::king].lsb_square();
		return king_squares;
	}();
	half_move_clock=history_data.half_move_clock;
	if(!was_whites_move)
		--full_move_clock;
	Side_position& side_to_unmove = sides[last_moved_side];
	Side_position& current_side_to_move = sides[side_to_move];
	Side_map<Fixed_capacity_vector<std::uint16_t,4>> added_features;
	Side_map<Fixed_capacity_vector<std::uint16_t,4>> removed_features;
	const Position& previous_move_destination = history_data.move.destination_square(), previous_move_origin = history_data.move.from_square();
	if(history_data.piece == Piece::pawn && previous_move_destination.rank_ == promotion_rank)
	{
		side_to_unmove.pieces[history_data.move.promotion_piece()].remove_piece(previous_move_destination);
		side_to_unmove.pieces[Piece::pawn].add_piece(previous_move_origin);
		for(const auto side : all_sides)
		{
			removed_features[side].push_back(Neural_network::compute_feature_index(history_data.move.promotion_piece(), previous_move_destination, king_squares[side], last_moved_side, side));
			added_features[side].push_back(Neural_network::compute_feature_index(Piece::pawn, previous_move_origin, king_squares[side], last_moved_side, side));
		}
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
		side_to_unmove.pieces[Piece::king].move_piece(previous_move_destination, previous_move_origin);
		side_to_unmove.pieces[Piece::rook].move_piece(rook_destination_square, rook_origin_square);
		// refresh for the last_moved_side so only update side_to_move
		removed_features[side_to_move].push_back(Neural_network::compute_feature_index(Piece::rook, rook_destination_square, king_squares[side_to_move], last_moved_side, side_to_move));
		added_features[side_to_move].push_back(Neural_network::compute_feature_index(Piece::rook, rook_origin_square, king_squares[side_to_move], last_moved_side, side_to_move));
	}
	else
	{
		if(history_data.was_en_passant)
		{
			const auto direction = was_whites_move? 1:-1;
			const Position pawn_to_return{previous_move_destination.rank_-direction, previous_move_destination.file_};
			current_side_to_move.pieces[Piece::pawn].add_piece(pawn_to_return);
			for(const auto side : all_sides)
				added_features[side].push_back(Neural_network::compute_feature_index(Piece::pawn, pawn_to_return, king_squares[side], last_moved_side, side));
		}
		side_to_unmove.pieces[history_data.piece].move_piece(previous_move_destination, previous_move_origin);
		if(history_data.piece!=Piece::king)
		{
			for(const auto side : all_sides)
			{
				added_features[side].push_back(Neural_network::compute_feature_index(history_data.piece, previous_move_origin, king_squares[side], last_moved_side, side));
				removed_features[side].push_back(Neural_network::compute_feature_index(history_data.piece, previous_move_destination, king_squares[side], last_moved_side, side));
			}
		}
	}

	if(history_data.captured_piece)
	{
		current_side_to_move.pieces[*history_data.captured_piece].add_piece(previous_move_destination);
		for(const auto side : all_sides)
			added_features[side].push_back(Neural_network::compute_feature_index(*history_data.captured_piece, previous_move_destination, king_squares[side], side_to_move, side));
	}

	update_accumulator(removed_features, added_features, last_moved_side, history_data.piece);

	sides[Side::white].castling_rights=history_data.white_castling_rights;
	sides[Side::black].castling_rights=history_data.black_castling_rights;


	side_to_move = last_moved_side;
	enemy_attack_map = history_data.enemy_attack_map;
	en_passant_target_square = history_data.en_passant_target_square;

	repetition_history.pop_back();
	zobrist_hash = history_data.previous_zobrist_hash;
	evaluation = history_data.evaluation;
}

Bitboard State::occupied_squares() const noexcept
{
	return sides[Side::black].occupied_squares() | sides[Side::white].occupied_squares();
}

bool State::in_check() const noexcept
{
	return !(enemy_attack_map & sides[side_to_move].pieces[Piece::king]).is_empty();
}

std::vector<State::Piece_and_data> State::get_board_data() const noexcept
{
	std::vector<Piece_and_data> board_data{};
	for(const auto& side : all_sides)
	{
		for(const auto& piece : all_pieces)
		{
			sides[side].pieces[piece].for_each_piece([&board_data, &side, &piece](const Position& position)
			{
				board_data.push_back(Piece_and_data{piece, position, side});
			});
		}
	}
	return board_data;
}

std::vector<std::uint16_t> State::to_halfKP_features(const Side perspective) const noexcept
{
	std::vector<std::uint16_t> active_feature_indexes;
	Position king_square{sides[perspective].pieces[Piece::king].lsb_square()};
	for(const auto current_side : all_sides)
	{
		for(const auto& piece : all_pieces)
		{
			if(piece==Piece::king)
				continue;

			sides[current_side].pieces[piece].for_each_piece([&](const Position& piece_square)
			{
				active_feature_indexes.push_back(Neural_network::compute_feature_index(piece, piece_square, king_square, current_side, perspective));
			});
		}
	}
	return active_feature_indexes;
}

double State::evaluate() const noexcept
{
	return neural_network.evaluate(side_to_move);
}
