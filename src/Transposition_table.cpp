#include "Transposition_table.h"

#include <random>

using namespace engine;

const zobrist::location_to_number_t zobrist::location_to_number = [&]()
{
    zobrist::location_to_number_t location_to_number;
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist;
    for(auto& side_container : location_to_number)
    {
        for(auto& position_container : side_container)
        {
            for(auto& position : position_container)
            {
                position = dist(rng);
            }
        }
    }
    return location_to_number;
}();

std::uint64_t zobrist::hash(State state) noexcept
{
    std::uint64_t hash{0};
    for(const auto& [piece, position, side] : state.get_board_data())
    {
        hash ^= location_to_number[piece][side][to_index(position)];
    }
}