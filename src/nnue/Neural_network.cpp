#include "Neural_network.h"

int Neural_network::evaluate(const engine::Side side_to_move) const noexcept
{
	std::array<std::int16_t, Feature_transformer::dimensions.neurons*2> transformed_features;
	std::ranges::copy(accumulator[side_to_move]
					, transformed_features.begin());
	std::ranges::copy(accumulator[other_side(side_to_move)]
					, transformed_features.begin()+Feature_transformer::dimensions.neurons);

	const auto d1_transformed{dense_one.transform(clipped_ReLU(transformed_features))};
	const auto d2_transformed{dense_two.transform(clipped_ReLU(d1_transformed,64))};
	const auto d3_transformed{dense_three.transform(clipped_ReLU(d2_transformed,64))};

	return d3_transformed.front();
}

void Neural_network::refresh_accumulator(std::span<const std::uint16_t> features, const engine::Side side) noexcept
{
	feature_transformer.transform(features, accumulator[side]);
}

void Neural_network::update_accumulator(std::span<const std::uint16_t> removed_features
									  , std::span<const std::uint16_t> added_features
									  , const engine::Side perspective) noexcept
{
	auto& side_accumulator{accumulator[perspective]};
	feature_transformer.adjust(removed_features, std::minus<>{}, side_accumulator);
	feature_transformer.adjust(added_features, std::plus<>{}, side_accumulator);
}
