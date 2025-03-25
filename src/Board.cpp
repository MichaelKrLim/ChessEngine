#include "Board.h"
#include "Move_generator.h"

#include <sstream>

using namespace engine;

std::stack<Board::State_delta> Board::history{};

Board::Board(const std::string_view& fen_string)
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
	const auto read_placement_data = [&](const std::string_view& fen_section)
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
	std::istringstream iss{std::string{fen_string}};
	Position en_passent_target_square;
	std::string fen_segment;
	std::getline(iss, fen_segment, ' ');
	read_placement_data(fen_segment);

	std::getline(iss, fen_segment, ' ');
	side_to_move = fen_segment[0] == 'w'? Side::white : Side::black;

	std::getline(iss, fen_segment, ' ');
	if(fen_segment[0] != '-')
	{
		for(const char& castling_right : fen_segment)
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

	std::getline(iss, fen_segment, ' ');
	if(fen_segment[0] != '-')
	{
		assert(fen_segment.size()==2 && isalpha(fen_segment[0]) && isdigit(fen_segment[1]) && "Invalid FEN string");
		en_passent_target_square = algebraic_to_position(fen_segment);
		std::size_t side_index;
		if(en_passent_target_square.rank_ == white_en_passant_target_rank-1)
			side_index = static_cast<std::uint8_t>(Side::white);
		else
			side_index = static_cast<std::uint8_t>(Side::black);
		sides[side_index].en_passent_target_square = en_passent_target_square;
	}

	std::getline(iss, fen_segment, ' ');
	half_move_clock = std::stoi(fen_segment);

	std::getline(iss, fen_segment, ' ');
	full_move_clock = std::stoi(fen_segment);

	enemy_attack_map = generate_attack_map(*this, !side_to_move);
}

void Board::make(const Move& move)
{
	const auto& history_size = history.size();
	Side_position& side = sides[static_cast<std::uint8_t>(side_to_move)];
	auto& opposite_side = sides[static_cast<std::uint8_t>(!side_to_move)];
	const auto destination_square = move.destination_square();
	const auto from_square = move.from_square();
	std::optional<Piece> piece_to_capture = std::nullopt;
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
				}
				found = true;
			});
			if(found) break;
		}
	}
	if(move.type() == Move_type::promotion)
	{
		history.push(State_delta{move, static_cast<Piece>(Piece::pawn), piece_to_capture, enemy_attack_map});
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
				if(occupied_square == from_square)
				{
					history.push(State_delta{move, static_cast<Piece>(i), piece_to_capture, enemy_attack_map});
					side.pieces[i] ^= (1ULL << to_index(occupied_square)) | (1ULL << to_index(destination_square));
					moved = true;
					return;
				}
			});
			if(moved) break;
		}
	}
	++half_move_clock;
	if(side_to_move == Side::black) ++full_move_clock;
	enemy_attack_map = generate_attack_map(*this, side_to_move);
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
	
	const auto [move, moved_piece, captured_piece, attack_map] = history.top();
	history.pop();
	const auto move_type = move.type();
	Side_position& side_to_unmove = sides[static_cast<std::uint8_t>(last_moved_side)];
	Side_position& current_side_to_move = sides[static_cast<std::uint8_t>(side_to_move)];
	if(move_type == Move_type::promotion)
	{
		side_to_unmove.pieces[static_cast<std::uint8_t>(move.promotion_piece())].remove_piece(move.destination_square());
		side_to_unmove.pieces[static_cast<std::uint8_t>(Piece::pawn)].add_piece(move.from_square());
	}
	if(move_type == Move_type::castling)
	{
		// TODO
	}
	else
	{
		if(move_type == Move_type::en_passant)
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
}

Bitboard Board::occupied_squares() const noexcept
{
	return sides[static_cast<std::uint8_t>(Side::black)].occupied_squares() |= sides[static_cast<std::uint8_t>(Side::white)].occupied_squares();
}

bool Board::in_check() const noexcept
{
	return is_square_attacked(sides[static_cast<std::uint8_t>(side_to_move)].pieces[static_cast<std::uint8_t>(Piece::king)].lsb_square());
}
