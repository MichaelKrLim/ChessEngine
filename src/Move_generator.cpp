#include "Move_generator.h"
#include "Position.h"
#include "Utility.h"

using namespace engine;

namespace std 
{
	template <>
	struct hash<Position>
	{
		size_t operator()(const Position& p) const
		{
			size_t h1 = hash<int>{}(p.rank_);
			size_t h2 = hash<int>{}(p.file_);
			return h1 ^ (h2 << 1);
		}
	};
}

template<std::size_t I, std::size_t ...Is>
consteval void permute(const auto&& add_blocker, const auto&& f, const std::size_t x_scalar, const std::size_t y_scalar)
{
	add_blocker(Position{x_scalar*I, y_scalar*I});
	f();
	permute<Is...>(add_blocker, f, x_scalar, y_scalar);
}

consteval void permute(const auto&& add_blocker, const auto&& f, const std::size_t x_scalar, const std::size_t y_scalar)
{
	const auto Is = std::make_index_sequence<board_size>{};
	permute(f, x_scalar, y_scalar, Is...);
}

template <std::size_t size>
consteval std::array<Bitboard, size> Move_generator::blocker_configurations(const Position& square, const bool& bishop)
{
	Bitboard current_configuration{};
	std::array<Bitboard, size> blocker_configurations{};
	const auto add_blocker = [&](const Position& blocker_position)
	{
		if(is_on_board(blocker_position))
			current_configuration |= to_index(blocker_position);
	};
	auto permute = [] (this auto&& rec, auto&& f, std::size_t x_scalar, std::size_t y_scalar, std::size_t i = 0, std::index_sequence<I...> = std::make_index_sequence<1, board_size>{})
	{
		add_blocker(Position{x_scalar*i, y_scalar*i});
		f();
		rec(f, x_scalar, y_scalar, I...);
	};
	permute(permute(permute(permute([&]()
	{
		static std::size_t index{0};
		blocker_configurations[index++] = current_configuration;
		current_configuration = 0;
	}, bishop? -1:0, bishop? 1:1), bishop? 1:0, bishop? 1:1), bishop? -1:1, bishop? -1:0), bishop? 1:1, bishop? -1:0);
	return blocker_configurations;
}

consteval std::array<Bitboard, 107520> Move_generator::create_attack_table()
{
	std::array<Bitboard, 107520> attack_table{};
	std::size_t index{0};
	auto consteval_for = [] <std::size_t... I> (this auto&& rec, auto&& f, std::size_t i, std::index_sequence<I...> = std::make_index_sequence<board_size>{})
	{
		f();
		rec(f, I...);
	};
	for(std::size_t rank{0}; rank<board_size; ++rank)
	{
		for(std::size_t file{0}; file<board_size; ++file)
		{
			constexpr Position current_square = Position{rank, file};
			constexpr std::size_t number_of_configurations = (1 << rook_rellevant_bits[to_index(current_square)]); //2^n
			for(const auto& blocker_configuration : blocker_configurations<number_of_configurations>(current_square, false))
			{
				attack_table[index] = rook_reachable_squares(current_square, blocker_configuration);
				++index;
			}
			constexpr std::size_t number_of_configurations = (1 << bishop_rellevant_bits[to_index(current_square)]);
			for(const auto& blocker_configuration : blocker_configurations<number_of_configurations>(current_square, true))
			{
				attack_table[index] = bishop_reachable_squares(current_square, blocker_configuration);
				++index;
			}
		}	
	}
	return attack_table;
}

constexpr std::array<Bitboard, 107520> Move_generator::attack_table_ = create_attack_table();

constexpr Move_generator::Move_generator()
{
	cast_magic();
}

const std::array<Move_generator::moves_type, number_of_piece_types> Move_generator::get(const Side_position& side, const Side& active_player) const
{
	std::array<moves_type, number_of_piece_types> legal_moves{};
	pawn_legal_moves(side.pieces[static_cast<std::size_t>(Piece::pawn)], side.occupied_squares, active_player);
	knight_legal_moves(side.pieces[static_cast<std::size_t>(Piece::knight)], side.occupied_squares);
}

const Move_generator::moves_type Move_generator::pawn_legal_moves(const Bitboard& pawns_bb, const Bitboard& occupied_squares, const Side& active_player) const
{
	moves_type legal_moves{};
	const auto rank = 0xFF;
	const auto pawn_direction = active_player == Side::white ? 1 : -1;
	const auto starting_rank = active_player == Side::white ? 1 : 6;
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

const Move_generator::moves_type Move_generator::knight_legal_moves(const Bitboard& knight_bb, const Bitboard& occupied_squares) const
{
	moves_type legal_moves;
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

const Move_generator::moves_type Move_generator::king_legal_moves(const Bitboard& king_bb, const Bitboard& occupied_squares) const
{
	moves_type legal_moves;
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

constexpr Bitboard Move_generator::rook_reachable_squares(const Position& original_square, const Bitboard& occupied_squares)
{
	Bitboard valid_moves{};
	for(int del_rank{0}; del_rank < board_size; ++del_rank)
	{
		for(int del_file{0}; del_file < board_size; ++del_file)
		{
			const Position destination_square = original_square + Position{del_rank, del_file};
			if(is_valid_destination(destination_square, occupied_squares))
				valid_moves |= (1 << to_index(destination_square));
			else 
				return valid_moves;
		}
	}
	return valid_moves;
}

constexpr Bitboard Move_generator::bishop_reachable_squares(const Position& bishop_square, const Bitboard& occupied_squares)
{
	const auto explore_diagonal = [](this auto&& rec, const Position& bishop_square, const Position& diagonal_offset,
		const Bitboard occupied_squares, Bitboard& valid_moves,
		const Position& original_bishop_square)
	{
		const Position destination_square = bishop_square + diagonal_offset;
		if(!is_valid_destination(destination_square, occupied_squares))
			return;
		valid_moves |= (1 << to_index(destination_square));

		rec(destination_square, diagonal_offset, occupied_squares, valid_moves, original_bishop_square);
	};

	Bitboard valid_moves{};
	for(const auto& direction : bishop_moves_)
	{
		explore_diagonal(bishop_square, direction, occupied_squares, valid_moves, bishop_square);
	}
	return valid_moves;
}

constexpr void Move_generator::cast_magic()
{

}
