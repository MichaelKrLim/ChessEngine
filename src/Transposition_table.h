#ifndef Transposition_table_h_INCLUDED
#define Transposition_table_h_INCLUDED

#include "Pieces.h"
#include "State.h"

#include <cstdint>

namespace zobrist
{
    using location_to_number_t = engine::Piece_map<engine::Side_map<std::array<std::uint64_t, 64>>>;
    const static location_to_number_t location_to_number;

    std::uint64_t hash(engine::State state) noexcept;
}

namespace engine
{
    enum class Search_decision
    {
        exact, upperbound, lowerbound
    };

    struct Transposition_data
    {
        Search_decision search_decision;
        unsigned int depth;
    };

    template <std::size_t table_size_log2_bytes>
    class Transposition_table
    {
        public:
        
        inline Transposition_data operator[](State state)
        {
            const auto index = zobrist::hash(state) % data.size();
            return data[index];
        }

        private:
        
        std::array<Transposition_data, 1ULL<<table_size_log2_bytes> data{};
    };
}

#endif