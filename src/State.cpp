#include "Constants.h"
#include "Fixed_capacity_vector.h"
#include "Move_generator.h"
#include "nnue/Neural_network.h"
#include "State.h"
#include "Transposition_table.h"

#include <algorithm>
#include <cstdint>
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

Fixed_capacity_vector<std::uint16_t, board_size*board_size> State::to_halfKP_features(const Side perspective) const noexcept
{
	Fixed_capacity_vector<std::uint16_t,board_size*board_size> active_feature_indexes;
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

int State::evaluate(const Neural_network& neural_network, const engine::Side_map<std::array<std::int16_t, Feature_transformer::dimensions.neurons>>& accumulator) const noexcept
{
	return neural_network.evaluate(side_to_move, accumulator);
}
