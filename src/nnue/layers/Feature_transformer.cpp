#include "Feature_transformer.h"

Feature_transformer::Feature_transformer(std::ifstream& net_file) noexcept
	: biases(dimensions.neurons)
	, data(dimensions.neurons*dimensions.features)
{
	hash=read_little_endian<decltype(hash)>(net_file);

	for(auto& bias : biases)
		bias=read_little_endian<bias_type>(net_file);

	weights_container weights(data.data());
	for(std::size_t feature_index{0}; feature_index<dimensions.features; ++feature_index)
	{
		for(std::size_t neuron{0}; neuron<dimensions.neurons; ++neuron)
			weights[neuron,feature_index]=read_little_endian<weight_type>(net_file);
	}
}

void Feature_transformer::transform(const std::vector<std::uint16_t>& active_feature_indexes
								  , std::span<bias_type, Feature_transformer::dimensions.neurons> transformed) const noexcept
{
	const_weights_container weights{weights_view()};
	for(std::size_t neuron_index{0}; neuron_index<dimensions.neurons; ++neuron_index)
	{
		const auto neuron{std::submdspan(weights, neuron_index, std::full_extent)};
		for(const auto& feature_index : active_feature_indexes)
		{
			transformed[neuron_index]+=neuron[feature_index];
		}
		transformed[neuron_index]+=biases[neuron_index];
	}
}
