#ifndef FEN_h_INCLUDED
#define FEN_h_INCLUDED

#include "Pieces.h"

#include <iostream>
#include <string>
#include <string_view>

namespace engine 
{
	class FEN
	{
		public:

		FEN() { std::cin >> state_string_; };
		explicit FEN(std::string state_string) : state_string_(state_string) {};
		
		[[nodiscard]] std::string_view state_string() const;
		[[nodiscard]] Piece to_piece(const char& to_convert) const;

		private:

		std::string state_string_;

	};
}

#endif // FEN_h_INCLUDED
