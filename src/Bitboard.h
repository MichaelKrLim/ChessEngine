#ifndef	Bitboard_h_INCLUDED
#define	Bitboard_h_INCLUDED

#include "Constants.h"

#include <bit>
#include <cstdint>
#include <ostream>
#include <string>

namespace engine
{
	struct Position;
	class Bitboard
	{
		public:

		explicit constexpr inline Bitboard(const std::uint64_t& data) : data_(data) {}
		inline Bitboard() = default;

		constexpr inline Bitboard operator& (const std::uint64_t& value) const { return Bitboard(data_ & value); }
		constexpr inline Bitboard operator& (const Bitboard& bitboard)   const { return Bitboard(data_ & bitboard.data_); }	
		constexpr inline Bitboard operator| (const Bitboard& bitboard)   const { return Bitboard(data_ | bitboard.data_); }
		constexpr inline Bitboard operator~()                            const { return Bitboard(~data_); }
		constexpr inline Bitboard operator<<(const std::uint64_t& value) const { return Bitboard(data_ << value); }
		
		constexpr inline Bitboard& operator|=(const std::size_t& value)  { data_ |= value; return *this; }
		constexpr inline Bitboard& operator|=(const Bitboard& bitboard)  { data_ |= bitboard.data_; return *this; }
		constexpr inline void      operator= (const std::uint64_t& data) { data_ = data; }

		constexpr inline bool operator> (const std::uint64_t& value) const { return data_ > value; }
		constexpr inline bool operator< (const std::uint64_t& value) const { return data_ < value; }
		constexpr inline bool operator==(const std::uint64_t& value) const { return data_ == value; }
		constexpr inline bool operator!=(const std::uint64_t& value) const { return data_ != value; }
		
		[[nodiscard]] constexpr bool is_occupied(const Position& position) const;
		[[nodiscard]] constexpr std::string pretty_string() const;
		[[nodiscard]] constexpr Position lsb_index() const;
		constexpr void hash(const int& magic);
		template <typename Callable>
		constexpr void for_each_piece(Callable&& f) const;

		private:

		std::uint64_t data_{0};

		constexpr friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);
	};
	
	template <typename Callable>
	constexpr void Bitboard::for_each_piece(Callable&& f) const 
	{
		auto data_c = data_;
		while(data_c > 0)
		{
			const auto index = std::countr_zero(data_c);
			f(index);
			data_c &= data_c - 1;
		}
	}

	constexpr std::string Bitboard::pretty_string() const
	{
		std::string s = "+---+---+---+---+---+---+---+---+\n";
		for(int r = 7;	r >= 0;	--r)
		{
			for(std::size_t f = 0; f <= 7; ++f)
				s += (data_ & (1ULL << (f+r*board_size)))? "| X " : "|   ";

			s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
		}
		s += "  a   b   c   d   e   f   g   h\n";
		return s;
	}

	constexpr bool Bitboard::is_occupied(const Position& square) const { return data_ & (1<<(square.rank_*board_size+square.file_)); }
	constexpr void Bitboard::hash(const int& magic) { data_ *= magic; }
	constexpr Position Bitboard::lsb_index() const { return Position{static_cast<std::size_t>(std::countr_zero(data_))}; }
	
	constexpr std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) { return os << bitboard.pretty_string(); }
}

#endif // Bitboard_h_INCLUDED
