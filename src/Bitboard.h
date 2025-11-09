#ifndef	Bitboard_h_INCLUDED
#define	Bitboard_h_INCLUDED

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

		explicit constexpr Bitboard(const std::uint64_t data) : data_(data) {}
		constexpr Bitboard() = default;

		explicit constexpr operator std::uint64_t() const { return data_; }

		constexpr Bitboard  operator&(const Bitboard& bitboard)  const  { return Bitboard{data_ & bitboard.data_}; }
		constexpr Bitboard  operator|(const Bitboard& bitboard)  const  { return Bitboard{data_ | bitboard.data_}; }
		constexpr Bitboard  operator^(const Bitboard& bitboard)  const  { return Bitboard{data_ ^ bitboard.data_}; }
		constexpr Bitboard  operator~()                          const  { return Bitboard{~data_};                 }

		constexpr Bitboard& operator^=(const Bitboard& bitboard)        { data_ ^= bitboard.data_; return *this; }
		constexpr Bitboard& operator|=(const Bitboard& bitboard)        { data_ |= bitboard.data_; return *this; }
		constexpr Bitboard& operator&=(const Bitboard& bitboard)        { data_ &= bitboard.data_; return *this; }

		template <typename Integral_type>
			requires std::integral<Integral_type>
		constexpr Bitboard& operator>>=(const Integral_type shift)      { data_ >>= shift; return *this; }
		template <typename Integral_type>
			requires std::integral<Integral_type>
		constexpr Bitboard& operator<<=(const Integral_type shift)      { data_ <<= shift; return *this; }
		template <typename Integral_type>
			requires std::integral<Integral_type>
		constexpr Bitboard  operator>>(const Integral_type shift) const { return Bitboard{data_ >> shift}; }
		template <typename Integral_type>
			requires std::integral<Integral_type>
		constexpr Bitboard  operator<<(const Integral_type shift) const { return Bitboard{data_ << shift}; }


		constexpr bool      operator==(const Bitboard& bitboard)   const { return data_ == bitboard.data_; }
		constexpr bool      operator!=(const Bitboard& bitboard)   const { return !(*this == bitboard); }

		[[nodiscard]] constexpr bool is_occupied(const Position& position) const;
		[[nodiscard]] std::string pretty_string() const;
		[[nodiscard]] constexpr Position lsb_square() const;
		[[nodiscard]] constexpr std::uint8_t popcount() const { return std::popcount(data_); }
		[[nodiscard]] constexpr bool is_empty() const { return data_ == 0; }
		constexpr void add_piece(const Position& index);
		constexpr void remove_piece(const Position& square);
		constexpr void move_piece(const Position& origin_square, const Position& destination_square);
		template <typename Function_type>
		constexpr void for_each_piece(Function_type&& f) const;

		[[nodiscard]] constexpr static Bitboard onebit(const Position& position) { return Bitboard{1ULL}<<to_index(position); }
		[[nodiscard]] constexpr static Bitboard all_bits() { return ~Bitboard{0ULL}; }
		[[nodiscard]] constexpr static Bitboard rank(const int rank_index) { return Bitboard{rank_one}<<(rank_index*board_size); }
		[[nodiscard]] constexpr static Bitboard file(const int file_index) { return Bitboard{file_a}<<file_index; }

		private:

		std::uint64_t data_{0};

		friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);
	};

	constexpr bool is_free(const Position& square, const Bitboard& occupied_squares)
	{
		return (occupied_squares & Bitboard::onebit(square)).is_empty();
	}

	constexpr void Bitboard::add_piece(const Position& square)
	{
		*this |= Bitboard::onebit(square);
	}

	constexpr void Bitboard::remove_piece(const Position& square)
	{
		*this &= ~Bitboard::onebit(square);
	}

	constexpr void Bitboard::move_piece(const Position& origin_square, const Position& destination_square)
	{
		*this ^= Bitboard::onebit(origin_square) | Bitboard::onebit(destination_square);
	}

	inline std::string Bitboard::pretty_string() const
	{
		std::string s{"+---+---+---+---+---+---+---+---+\n"};
		for(int r{7}; r>=0; --r)
		{
			for(std::size_t f{0}; f<=7; ++f)
				s+=(data_ & (1ULL << (f+r*board_size)))? "| X " : "|   ";
			s += "| " + std::to_string(1+r) + "\n+---+---+---+---+---+---+---+---+\n";
		}
		s += "  a   b   c   d   e   f   g   h\n";
		return s;
	}

	template <typename Function_type>
	inline constexpr void Bitboard::for_each_piece(Function_type&& f) const
	{
		auto data_c = data_;
		while(data_c > 0)
		{
			const std::size_t index = std::countr_zero(data_c);
			std::forward<Function_type>(f)(Position{index});
			data_c &= data_c - 1;
		}
	}

	constexpr bool Bitboard::is_occupied(const Position& square) const { return data_ & (1ULL << to_index(square)); }
	
	constexpr Position Bitboard::lsb_square() const { return Position{static_cast<std::size_t>(std::countr_zero(data_))}; }

	inline std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) { return os << bitboard.pretty_string(); }
}

#endif // Bitboard_h_INCLUDED
