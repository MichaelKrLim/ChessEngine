#ifndef	Bitboard_h_INCLUDED
#define	Bitboard_h_INCLUDED

#include "Position.h"

#include <cstdint>
#include <string>

namespace engine
{
	class Bitboard
	{
		public:

		explicit inline Bitboard(const std::uint64_t& data) : data_(data) {}
		inline Bitboard() = default;

		inline Bitboard operator&(const unsigned int& value) const { return Bitboard(data_ & value); }
		inline Bitboard operator|(const Bitboard& bitboard) const { return Bitboard(data_ | bitboard.data_); }
		
		inline Bitboard& operator=(const std::uint64_t& data) { data_ = data; return *this; }
		inline Bitboard& operator|=(const unsigned int& value) { data_ |= value; return *this; }
		inline Bitboard& operator|=(const Bitboard& bitboard) { data_ |= bitboard.data_; return *this; }
		
		inline bool operator>(const unsigned int& value) const { return data_ > value; }

		bool is_occupied(const Position& square) const;
		bool is_occupied(const std::uint64_t& position) const;
		void hash(const int& magic);
		std::string pretty_string() const;

		private:

		std::uint64_t data_;
	};
}

#endif // Bitboard_h_INCLUDED
