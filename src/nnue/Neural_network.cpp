#include "Neural_network.h"

#include <fstream>

Neural_network Neural_network::load_from_file(const std::filesystem::path& path) noexcept
{
	std::ifstream net_file{path};
	return Neural_network(net_file);
}

Neural_network::Neural_network(const std::filesystem::path& path)
	: Neural_network(load_from_file(path))
{}

Neural_network::Neural_network(std::istream& is)
	: header(is)
	, feature_transformer(is)
	, dense_layers_hash(read_little_endian<decltype(dense_layers_hash)>(is))
	, dense_one(is)
	, dense_two(is)
	, dense_three(is)
{}

int Neural_network::evaluate(const engine::Side side_to_move, const engine::Side_map<std::array<std::int16_t, Feature_transformer::dimensions.neurons>>& accumulator) const noexcept
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
