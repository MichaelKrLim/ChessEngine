#ifndef	Bitboard_h_INCLUDED
#define	Bitboard_h_INCLUDED

#include <bit>
#include <bitset>
#include <cstdint>
#include <ostream>
#include <string>

namespace engine
{
	class Position;
	class Bitboard
	{
		public:

		explicit inline Bitboard(const std::uint64_t& data) : data_(data) {}
		inline Bitboard() = default;

		inline Bitboard operator& (const std::uint64_t& value) const { return Bitboard(data_ & value); }
		inline Bitboard operator& (const Bitboard& bitboard)   const { return Bitboard(data_ & bitboard.data_); }	
		inline Bitboard operator| (const Bitboard& bitboard)   const { return Bitboard(data_ | bitboard.data_); }
		inline Bitboard operator~()                            const { return Bitboard(~data_); }
		inline Bitboard operator<<(const std::uint64_t& value) const { return Bitboard(data_ << value); }
		
		inline Bitboard& operator|=(const std::uint64_t& value) { data_ |= value; return *this; }
		inline Bitboard& operator|=(const Bitboard& bitboard)   { data_ |= bitboard.data_; return *this; }

		inline bool operator> (const std::uint64_t& value) const { return data_ > value; }
		inline bool operator< (const std::uint64_t& value) const { return data_ < value; }
		inline bool operator==(const std::uint64_t& value) const { return data_ == value; }
		inline bool operator!=(const std::uint64_t& value) const { return data_ != value; }
		
		bool is_occupied(const Position& square) const;
		bool is_occupied(const std::uint64_t& position) const;
		void hash(const int& magic);
		std::string pretty_string() const;
		template <typename Callable>
		void for_each_piece(Callable&& f) const;
		Position lsb_index() const;

		private:

		std::uint64_t data_{0};

		friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);
	};
	
	template <typename Callable>
	void Bitboard::for_each_piece(Callable&& f) const 
	{
		auto data_c = data_;
		while(data_c > 0)
		{
			const auto index = std::countr_zero(data_c);
			f(index);
			data_c &= data_c - 1;
		}
	}

	inline std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) { return os << std::bitset<64>(bitboard.data_); }
}

#endif // Bitboard_h_INCLUDED
