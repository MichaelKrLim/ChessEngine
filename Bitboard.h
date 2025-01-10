#ifndef Bitboard_h_INCLUDED
#define Biboard_h_INCLUDED

namespace engine
{
    class Bitboard
    {
        public:

        inline Bitboard(std::uint64_t& data) const : data_(data) {}
        Bitboard() = default;

        inline operator std::uint64_t() const { return data_; }

        inline Bitboard& operator=(std::uint64_t data) const { data_ = data; return *this; }

        bool is_occupied(const Position& square) const
	    std::string pretty_string() const

        private:

        std::uint64_t data_;
    }
}

#endif // Bitboard_h_INCLUDED