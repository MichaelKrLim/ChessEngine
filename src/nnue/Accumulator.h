#ifndef Accumulator_h_INCLUDED
#define Accumulator_h_INCLUDED

#include "../Constants.h"
#include "layers/Feature_transformer.h"
#include "Neural_network.h"
#include "../State.h"

#include <cstdint>
#include <span>

using Accumulator=engine::Side_map<std::array<std::int16_t, Feature_transformer::dimensions.neurons>>;

inline void refresh_accumulator(std::span<const std::uint16_t> features
					   , const engine::Side side
					   , const Neural_network& neural_network
					   , Accumulator& accumulator) noexcept
{
	neural_network.transform_features(features, accumulator[side]);
}

inline Accumulator fresh_accumulator(const engine::State& state, const Neural_network& neural_network)
{
	Accumulator accumulator{};
	for(const auto side : engine::all_sides)
		refresh_accumulator(state.to_halfKP_features(side), side, neural_network, accumulator);
	return accumulator;
}

inline void update_accumulator(std::span<const std::uint16_t> removed_features
							 , std::span<const std::uint16_t> added_features
							 , const engine::Side perspective
							 , const Neural_network& neural_network
							 , Accumulator& accumulator) noexcept
{
	auto& side_accumulator{accumulator[perspective]};
	neural_network.adjust_accumulator(removed_features, std::minus<>{}, side_accumulator);
	neural_network.adjust_accumulator(added_features, std::plus<>{}, side_accumulator);
}

inline void change_accumulator(const engine::State& state
					  , const auto& removed_features
					  , const auto& added_features
					  , const engine::Side moved_side
					  , const engine::Piece moved_piece
					  , const Neural_network& neural_network
					  , Accumulator& accumulator) noexcept
{
	const engine::Side enemy_side{other_side(moved_side)};
	update_accumulator(removed_features[enemy_side], added_features[enemy_side], enemy_side, neural_network, accumulator);
	if(moved_piece==engine::Piece::king)
		refresh_accumulator(state.to_halfKP_features(moved_side), moved_side, neural_network, accumulator);
	else
		update_accumulator(removed_features[moved_side], added_features[moved_side], moved_side, neural_network, accumulator);
}

#endif // Accumulator_h_INCLUDED
