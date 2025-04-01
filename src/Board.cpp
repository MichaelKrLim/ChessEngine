#include "Board.h"
#include "Move_generator.h"

#include <algorithm>
#include <ranges>
#include <regex>
#include <sstream>

using namespace engine;

std::stack<Board::State_delta> Board::history{};

void Board::validate_fen(const std::array<std::string, 6>& partitioned_fen) const
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
		if(!std::regex_match(fen_active_color, std::regex{"^(w|b)$"}))
			throw std::invalid_argument("invalid active color");
		if(!std::regex_match(fen_castling_availiability, std::regex{"^-$|^(KQ?k?q?|Qk?q?|kq?|q)$"}))
			throw std::invalid_argument("invalid castling rights");
		if(!std::regex_match(fen_en_passant_target_square, std::regex{"^(-|[a-h][36])$"}))
			throw std::invalid_argument("invalid en passant square");
		if(!std::regex_match(fen_halfmove_clock, std::regex{"^([0-9]|[1-9][0-9])$"}))
			throw std::invalid_argument("invalid halfmove clock");
		if(!std::regex_match(fen_fullmove_clock, std::regex{"^([1-9][0-9]{0,1})$"}))
			throw std::invalid_argument("invalid halfmove clock");
	}
}

void Board::parse_fen(const std::string_view fen) noexcept
{
	const auto to_piece_index = [](const char& to_convert)
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
		return static_cast<std::size_t>(to_piece[to_convert]);
	};
	const auto to_shift = [](std::size_t board_index) -> std::size_t
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
				const auto side_index = static_cast<std::uint8_t>(side);
				std::size_t piece_type_index = to_piece_index(std::tolower(fen_section[i]));
				const std::uint64_t mask = (1ULL << to_shift(board_index));
				sides[side_index].pieces[piece_type_index] |= mask;
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
			std::uint8_t side_index, castling_right_index;
			if(std::isupper(castling_right))
				side_index = static_cast<std::uint8_t>(Side::white);
			else
				side_index = static_cast<std::uint8_t>(Side::black);
			if(std::tolower(castling_right) == 'k')
				castling_right_index = static_cast<std::uint8_t>(Castling_rights::kingside);
			else
				castling_right_index = static_cast<std::uint8_t>(Castling_rights::queenside);
			sides[side_index].castling_rights[castling_right_index] = true;
		}
	}
	if(fen_en_passant_target_square[0] != '-')
		en_passant_target_square = algebraic_to_position(fen_en_passant_target_square);
	half_move_clock = std::stoi(fen_halfmove_clock);
	full_move_clock = std::stoi(fen_fullmove_clock);
}

Board::Board(const std::string_view fen)
{
	parse_fen(fen);
	side_to_move = !side_to_move;
	enemy_attack_map = generate_attack_map(*this);
	side_to_move = !side_to_move;
}

std::optional<Piece> Board::piece_at(const Position& position, const Side& side) const noexcept
{
	std::optional<Piece> found_piece{std::nullopt};
	for(std::uint8_t piece_index{0}; piece_index < number_of_pieces; ++piece_index)
	{
		sides[static_cast<std::uint8_t>(side)].pieces[piece_index].for_each_piece([&](const Position& occupied_square) mutable
		{
			if(occupied_square == position)
			{
				found_piece = static_cast<Piece>(piece_index);
				return;
			}
		});
		if(found_piece.has_value())
			return found_piece;
	}
	return std::nullopt;
}

void Board::make(const Move& move)
{
	const auto& history_size = history.size();
	Side_position& side = sides[static_cast<std::uint8_t>(side_to_move)];
	auto& opposite_side = sides[static_cast<std::uint8_t>(!side_to_move)];
	const auto destination_square = move.destination_square();
	const auto from_square = move.from_square();
	const auto pawn_direction = side_to_move == Side::white ? 1 : -1;
	std::optional<Piece> piece_to_capture = std::nullopt;
	const auto old_en_passant_target_square = en_passant_target_square;
	en_passant_target_square = std::nullopt;
	if(!is_free(destination_square, occupied_squares()))
	{
		for(std::size_t i{0}; i<opposite_side.pieces.size(); ++i)
		{
			auto& piece_bb = opposite_side.pieces[i];
			bool found{false};
			piece_bb.for_each_piece([&destination_square, &piece_to_capture, &i, &found, &piece_bb](const Position& occupied_square) mutable
			{
				if(occupied_square == destination_square)
				{
					piece_to_capture = static_cast<Piece>(i);
					piece_bb.remove_piece(destination_square);
					found = true;
				}
			});
			if(found) break;
		}
	}
	if(false/*is promotion*/)
	{
		history.push(State_delta{move, static_cast<Piece>(Piece::pawn), piece_to_capture, enemy_attack_map, old_en_passant_target_square, false});
		Bitboard& pawn_bb = side.pieces[static_cast<std::uint8_t>(Piece::pawn)];
		pawn_bb.remove_piece(from_square);
		side.pieces[static_cast<std::uint8_t>(move.promotion_piece())].add_piece(destination_square);
	}
	else
	{
		for(std::uint8_t i{0}; i<side.pieces.size(); ++i)
		{
			bool moved{false};
			side.pieces[i].for_each_piece([&](const Position& occupied_square) mutable
			{
				const Piece piece_type = static_cast<Piece>(i);
				if(occupied_square == from_square)
				{
					if(piece_type  == Piece::pawn && destination_square.rank_ == from_square.rank_ + 2*pawn_direction)
					{
						en_passant_target_square = Position{from_square.rank_+pawn_direction, from_square.file_};
					}
					const bool is_en_passant = destination_square==old_en_passant_target_square && piece_type == Piece::pawn;
					if(is_en_passant)
					{
						opposite_side.pieces[static_cast<std::uint8_t>(Piece::pawn)].remove_piece(Position{destination_square.rank_-pawn_direction, destination_square.file_});
					}
					history.push(State_delta{move, static_cast<Piece>(i), piece_to_capture, enemy_attack_map, old_en_passant_target_square, is_en_passant});
					side.pieces[i] ^= (1ULL << to_index(occupied_square)) | (1ULL << to_index(destination_square));
					moved = true;
					return;
				}
			});
			if(moved)
				break;
		}
	}
	++half_move_clock;
	if(side_to_move == Side::black) ++full_move_clock;
	enemy_attack_map = generate_attack_map(*this);
	side_to_move = !side_to_move;
	assert(history.size() > history_size);
}

void Board::unmove()
{
	assert(history.size() > 0 && "Tried to undo noexistent move");
	const bool was_whites_move = side_to_move == Side::black;
	const auto last_moved_side = was_whites_move? Side::white : Side::black;
	
	--half_move_clock;
	if(!was_whites_move) --full_move_clock;
	
	const auto [move, moved_piece, captured_piece, attack_map, previous_en_passant_target_square, was_en_passant] = history.top();
	history.pop();
	Side_position& side_to_unmove = sides[static_cast<std::uint8_t>(last_moved_side)];
	Side_position& current_side_to_move = sides[static_cast<std::uint8_t>(side_to_move)];
	if(false /*is promotion*/)
	{
		side_to_unmove.pieces[static_cast<std::uint8_t>(move.promotion_piece())].remove_piece(move.destination_square());
		side_to_unmove.pieces[static_cast<std::uint8_t>(Piece::pawn)].add_piece(move.from_square());
	}
	if(false /*is castling*/)
	{
		// TODO
	}
	else
	{
		if(was_en_passant)
		{
			const auto direction = was_whites_move? 1 : -1;
			const Position destination_square = move.destination_square();
			const Position pawn_to_return = Position{destination_square.rank_-direction,destination_square.file_};
			current_side_to_move.pieces[static_cast<std::uint8_t>(Piece::pawn)].add_piece(pawn_to_return);
		}
		side_to_unmove.pieces[static_cast<std::uint8_t>(moved_piece)] ^= (1ULL << to_index(move.destination_square())) | (1ULL << to_index(move.from_square()));
		if(captured_piece)
			current_side_to_move.pieces[static_cast<std::uint8_t>(captured_piece.value())].add_piece(move.destination_square());
	}
	side_to_move = last_moved_side;
	enemy_attack_map = attack_map;
	en_passant_target_square = previous_en_passant_target_square;
}

Bitboard Board::occupied_squares() const noexcept
{
	return sides[static_cast<std::uint8_t>(Side::black)].occupied_squares() |= sides[static_cast<std::uint8_t>(Side::white)].occupied_squares();
}

bool Board::in_check() const noexcept
{
	return is_square_attacked(sides[static_cast<std::uint8_t>(side_to_move)].pieces[static_cast<std::uint8_t>(Piece::king)].lsb_square());
}
