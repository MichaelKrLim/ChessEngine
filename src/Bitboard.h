#ifndef	Bitboard_h_INCLUDED
#define	Bitboard_h_INCLUDED

#include "Constants.h"
#include "Position.h"

#include <bit>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace engine
{
	struct Bitboard
	{
		public:

		explicit inline constexpr Bitboard(const std::uint64_t& data) : data_(data) {}
		inline constexpr Bitboard() = default;

		explicit operator size_t() const { return static_cast<size_t>(data_); }

		inline constexpr Bitboard 		operator& (std::uint64_t value) 	 const { return Bitboard(data_ & value); }
		inline constexpr Bitboard 		operator& (const Bitboard& bitboard) const { return Bitboard(data_ & bitboard.data_); }	
		inline constexpr Bitboard 		operator| (const Bitboard& bitboard) const { return Bitboard(data_ | bitboard.data_); }
		inline constexpr Bitboard 		operator~ () 						 const { return Bitboard(~data_); }
		inline constexpr Bitboard 		operator<<(std::uint64_t shift) 	 const { return Bitboard(data_ << shift); }
		inline constexpr Bitboard 		operator>>(std::uint64_t shift) 	 const { return Bitboard(data_ >> shift); }
		inline constexpr std::uint64_t  operator*(std::uint64_t multiplier)  const { return data_*multiplier; }
		
		inline constexpr Bitboard& operator|= (std::size_t value) 		 { data_ |= value; return *this; }
		inline constexpr Bitboard& operator|= (const Bitboard& bitboard) { data_ |= bitboard.data_; return *this; }
		inline constexpr Bitboard& operator>>=(std::uint64_t shift) 	 { data_ = data_ >> shift; return *this; }
		inline constexpr Bitboard& operator&= (std::uint64_t value) 	 { data_ = data_ & value; return *this; }
		inline constexpr void      operator=  (std::uint64_t data)  	 { data_ = data; }

		inline constexpr bool operator> (std::uint64_t value) const { return data_ > value; }
		inline constexpr bool operator< (std::uint64_t value) const { return data_ < value; }
		inline constexpr bool operator==(std::uint64_t value) const { return data_ == value; }
		inline constexpr bool operator!=(Bitboard bitboard)   const { return data_ == bitboard.data_; };
		inline constexpr bool operator!=(std::uint64_t value) const { return data_ != value; }

		[[nodiscard]] constexpr bool is_occupied(const Position& position) const;
		[[nodiscard]] std::string pretty_string() const;
		[[nodiscard]] constexpr Position lsb_index() const;
		constexpr void for_each_piece(std::function<void (const Position& original_square)>&& f) const;

		private:

		std::uint64_t data_{0};

		friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);
	};
	
	inline constexpr void Bitboard::for_each_piece(std::function<void (const Position& original_square)>&& f) const 
	{
		auto data_c = data_;
		while(data_c > 0)
		{
			const std::size_t index = std::countr_zero(data_c);
			f(Position{index});
			data_c &= data_c - 1;
		}
	}

	inline std::string Bitboard::pretty_string() const
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

	inline constexpr bool        Bitboard::is_occupied(const Position& square) const { return data_ & (1<<(square.rank_*board_size+square.file_)); }
	inline constexpr Position    Bitboard::lsb_index() 						   const { return Position{static_cast<std::size_t>(std::countr_zero(data_))}; }

	inline std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) { return os << bitboard.pretty_string(); }
}

#endif // Bitboard_h_INCLUDED
