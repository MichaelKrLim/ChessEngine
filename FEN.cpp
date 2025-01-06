#include "FEN.h"

#include <array>

using namespace engine;

std::string_view FEN::state_string() const
{
	return state_string_;
}

Piece FEN::to_piece(const char& to_convert) const
{	
	constexpr std::array<Piece, 256> to_piece = []()
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
}

FEN FEN::from_input()
{
	std::string state_string;
	std::cin >> state_string;
	return FEN(std::move(state_string));
}
