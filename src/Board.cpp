#include "Board.h"

#include <sstream>

using namespace engine;

std::stack<Board> Board::history{};

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
}


void Board::make(const Move& move)
{
	history.push(move);
	Side_position& side = sides[static_cast<std::uint8_t>(side_to_move)];
	auto& opposite_side = sides[static_cast<std::uint8_t>(side_to_move == Side::white? Side::black : Side::white)];
	const auto destination_square = move.destination_square();
	const auto from_square = move.from_square();
	if(move.move_type() == Move_type::promotion)
	{
		Bitboard& pawn_bb = side.pieces[static_cast<std::uint8_t>(Piece::pawn)];
		pawn_bb.remove_piece(from_square);
		side.pieces[static_cast<std::uint8_t>(move.promotion_piece())].add_piece(destination_square);
	}
	else
	{
		if(!is_free(destination_square, occupied_squares()))
		{
			for(Bitboard& piece_bb : opposite_side.pieces)
			{
				bool found{false};
				piece_bb.for_each_piece([&destination_square, &found, &piece_bb](const Position& occupied_square) mutable
				{
					if(occupied_square == destination_square)
					{
						piece_bb.remove_piece(destination_square);
					}
					found = true;
					return;
				});
				if(found) break;
			}
		}
		for(Bitboard& piece_bb : side.pieces)
		{
			bool moved{false};
			piece_bb.for_each_piece([&](const Position& occupied_square) mutable
			{
				if(occupied_square == from_square)
				{
					piece_bb ^= (1ULL << to_index(occupied_square)) | (1ULL << to_index(destination_square));
					moved = true;
					return;
				}
			});
			if(moved) break;
		}
	}
	++half_move_clock;
	if(side_to_move == Side::black) ++full_move_clock;
	side_to_move = side_to_move == Side::white? Side::black : Side::white;
}

void Board::unmove()
{
	const bool is_white_to_move = side_to_move == Side::white;
	const Move move = history.top();
	history.pop();
	const auto move_type = move.type();
	--half_move_clock;
	if(is_white_to_move) --full_move_clock;
	side_to_move = is_white_to_move? Side::black : Side::white;
	if(move.type() == Move_type::promotion)
	{
		Side_position& moved_side = sides[static_cast<std::uint8_t>(side_to_move)];
		moved_side.pieces[static_cast<std::uint8_t>(move.promotion_piece())].remove_piece(move.destination_square());
		moved_side.pieces[static_cast<std::uint8_t>(Piece::pawn)].add_piece(move.from_square());
	}
}

Bitboard Board::occupied_squares() const noexcept
{
	return sides[static_cast<std::uint8_t>(Side::black)].occupied_squares() |= sides[static_cast<std::uint8_t>(Side::white)].occupied_squares();
}
