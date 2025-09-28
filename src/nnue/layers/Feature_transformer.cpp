#include "Feature_transformer.h"

Feature_transformer::Feature_transformer(std::ifstream& net_file) noexcept
{
	biases.resize(dimensions.neurons);
	weights.resize(dimensions.neurons, std::vector<weight_type>(dimensions.features));

	hash=read_little_endian<decltype(hash)>(net_file);

	for(auto& bias : biases)
		bias=read_little_endian<bias_type>(net_file);

	for(auto& row : weights)
	{
		for(auto& weight : row)
			weight=read_little_endian<weight_type>(net_file);
		//constexpr std::size_t to_skip{(max_simd_width-((sizeof(weight_type)*layer_dimensions.neurons)%max_simd_width))%max_simd_width};
		//net_file.ignore(to_skip);
	}
}


std::vector<Feature_transformer::bias_type> Feature_transformer::transform(const std::vector<std::int16_t>& active_feature_indexes) const noexcept
{
	std::vector<bias_type> transformed(dimensions.neurons);
	for(std::size_t neuron_index{0}; neuron_index<weights.size(); ++neuron_index)
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
