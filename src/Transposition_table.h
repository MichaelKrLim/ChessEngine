#ifndef Transposition_table_h_INCLUDED
#define Transposition_table_h_INCLUDED

#include "Pieces.h"
#include "State.h"

#include <array>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace zobrist
{
    using location_to_number_t = engine::Piece_map<engine::Side_map<std::array<std::uint64_t, 64>>>;
    const static location_to_number_t location_to_number = []()
    {
        zobrist::location_to_number_t location_to_number;
        std::mt19937_64 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;
        for (auto &side_container : location_to_number)
        {
            for (auto &position_container : side_container)
            {
                for (auto &position : position_container)
                {
                    position = dist(rng);
                }
            }
        }
        return location_to_number;
    }();

    [[nodiscard]] std::uint64_t hash(const engine::State& state) noexcept
    {
        std::uint64_t hash{0};
        for(const auto &[piece, position, side] : state.get_board_data())
        {
            hash ^= location_to_number[piece][side][to_index(position)];
        }
        return hash;
    }
}

namespace engine
{
    struct Transposition_data
    {
        unsigned depth;
        double eval;
        engine::Side to_move;
    };

    class Transposition_table
    {
        public:
        
        [[nodiscard]] std::optional<Transposition_data>& operator[](const State& state)
        {
            const auto index = zobrist::hash(state) % data.size();
            return data[index];
        }

        explicit Transposition_table(const unsigned table_size_log2) : data(1ULL<<table_size_log2, std::nullopt) {};
        
        private:
   
        std::vector<std::optional<Transposition_data>> data;
    };
}

#endif