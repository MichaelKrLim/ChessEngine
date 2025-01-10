#ifndef Bitboard_h_INCLUDED
#define Biboard_h_INCLUDED

namespace engine
{
    class Bitboard
    {
        public:

        inline Bitboard(const Bitboard& bb) const : data_(bb) {}
        Biboard() = default;

        inline operator std::uint64_t() const { return value; }

        inline Bitboard& operator=(std::uint64_t data) { data_ = data; return *this; }
        inline 

        bool is_occupied(const std::uint64_t bitboard, const Position& square) const

	    std::string pretty_string() 

        private:

        std::uint64_t data_;
    }
}

#endif // Bitboard_h_INCLUDED