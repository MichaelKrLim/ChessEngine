#ifndef	Bitboard_h_INCLUDED
#define	Bitboard_h_INCLUDED

#include "Constants.h"
#include "Position.h"

#include <bit>
#include <cstdint>
#include <ostream>
#include <string>

namespace engine
{
	struct Bitboard
	{
		public:

		explicit constexpr Bitboard(const std::uint64_t& data) : data_(data) {}
		constexpr Bitboard() = default;

		explicit operator size_t() const { return static_cast<size_t>(data_); }

		constexpr Bitboard 		operator& (std::uint64_t value) 	  const { return Bitboard(data_ & value); }
		constexpr Bitboard 		operator& (const Bitboard& bitboard)  const { return Bitboard(data_ & bitboard.data_); }
		constexpr Bitboard 		operator| (const Bitboard& bitboard)  const { return Bitboard(data_ | bitboard.data_); }
		constexpr Bitboard 		operator~ () 						  const { return Bitboard(~data_); }
		constexpr Bitboard 		operator<<(std::uint64_t shift) 	  const { return Bitboard(data_ << shift); }
		constexpr Bitboard 		operator>>(std::uint64_t shift) 	  const { return Bitboard(data_ >> shift); }
		constexpr std::uint64_t  operator* (std::uint64_t multiplier) const { return data_*multiplier; }

		constexpr Bitboard& operator^= (const std::uint64_t& u64) { data_ ^= u64; return *this; }
		constexpr Bitboard& operator|= (std::size_t value) 		  { data_ |= value; return *this; }
		constexpr Bitboard& operator|= (const Bitboard& bitboard) { data_ |= bitboard.data_; return *this; }
		constexpr Bitboard& operator>>=(std::uint64_t shift) 	  { data_ = data_ >> shift; return *this; }
		constexpr Bitboard& operator&= (std::uint64_t value) 	  { data_ = data_ & value; return *this; }
		constexpr Bitboard& operator&= (const Bitboard& bitboard) { data_ = data_ & bitboard.data_; return *this; }
		constexpr void      operator=  (std::uint64_t data)  	  { data_ = data; }

		constexpr bool operator> (std::uint64_t value) const { return data_ > value; }
		constexpr bool operator< (std::uint64_t value) const { return data_ < value; }
		constexpr bool operator==(std::uint64_t value) const { return data_ == value; }
		constexpr bool operator! ()                    const { return data_ == 0; }
		constexpr bool operator!=(Bitboard bitboard)   const { return data_ != bitboard.data_; };
		constexpr bool operator!=(std::uint64_t value) const { return data_ != value; }

		[[nodiscard]] constexpr bool is_occupied(const Position& position) const;
		[[nodiscard]] std::string pretty_string() const;
		[[nodiscard]] constexpr Position lsb_square() const;
		[[nodiscard]] constexpr std::uint8_t popcount() const { return std::popcount(data_); }
		constexpr void add_piece(const Position& index);
		constexpr void remove_piece(const Position& square);
		template <typename Function_type>
		constexpr void for_each_piece(Function_type&& f) const
		{
			auto data_c = data_;
			while(data_c > 0)
			{
				const std::size_t index = std::countr_zero(data_c);
				std::forward<Function_type>(f)(Position{index});
				data_c &= data_c - 1;
			}
		}

		private:

		std::uint64_t data_{0};

		friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);
	};

	constexpr bool is_free(const Position& square, const Bitboard& occupied_squares)
	{
		return (occupied_squares & (1ULL << to_index(square))) == 0;
	}

	constexpr Bitboard rank_bb(int rank) 
	{
		return Bitboard{0xFFULL << (rank * 8)};
	}

	constexpr void Bitboard::add_piece(const Position& square)
	{
		data_ |= (1ULL << to_index(square));
	}

	constexpr void Bitboard::remove_piece(const Position& square)
	{
		data_ &= ~(1ULL << to_index(square));
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

	constexpr bool        Bitboard::is_occupied(const Position& square) const { return data_ & (1ULL << to_index(square)); }
	
	constexpr Position    Bitboard::lsb_square() 						const { return Position{static_cast<std::size_t>(std::countr_zero(data_))}; }

	inline std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) { return os << bitboard.pretty_string(); }
}

#endif // Bitboard_h_INCLUDED
