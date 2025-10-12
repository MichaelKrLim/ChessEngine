#include "Feature_transformer.h"

Feature_transformer::Feature_transformer(std::ifstream& net_file) noexcept
{
	biases.resize(dimensions.neurons);
	weights.resize(dimensions.neurons, std::vector<weight_type>(dimensions.features));

	hash=read_little_endian<decltype(hash)>(net_file);

	for(auto& bias : biases)
		bias=read_little_endian<bias_type>(net_file);

	for(std::size_t feature_index{0}; feature_index<dimensions.features; ++feature_index)
	{
		for(std::size_t neuron{0}; neuron<dimensions.neurons; ++neuron)
			weights[neuron][feature_index]=read_little_endian<weight_type>(net_file);
	}
}

std::vector<Feature_transformer::bias_type> Feature_transformer::transform(const std::vector<std::uint16_t>& active_feature_indexes) const noexcept
{
	std::vector<bias_type> transformed(dimensions.neurons);
	for(std::size_t neuron_index{0}; neuron_index<dimensions.neurons; ++neuron_index)
	{
		const auto& neuron{weights[neuron_index]};
		for(const auto& feature_index : active_feature_indexes)
		{
			transformed[neuron_index]+=neuron[feature_index];
		}
		transformed[neuron_index]+=biases[neuron_index];
	}
	return transformed;
}
