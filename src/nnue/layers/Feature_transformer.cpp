#include "Feature_transformer.h"

#include <experimental/simd>
#include <numeric>

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

void Feature_transformer::transform(std::span<const std::uint16_t> active_feature_indexes
								  , std::span<bias_type, Feature_transformer::dimensions.neurons> transformed) const noexcept
{
	const_weights_container weights{weights_view()};
	for(std::size_t neuron_index{0}; neuron_index<dimensions.neurons; ++neuron_index)
	{
		const auto neuron{std::submdspan(weights, neuron_index, std::full_extent)};
		std::experimental::native_simd<weight_type> chunk;
		const std::size_t number_of_chunks{active_feature_indexes.size()/chunk.size()};
		for(std::size_t processed_chunks{0}; processed_chunks<number_of_chunks; ++processed_chunks)
		{
			for(std::size_t i{0}; i<chunk.size(); ++i)
				chunk[i]=neuron[active_feature_indexes[i+processed_chunks*chunk.size()]];
			transformed[neuron_index]+=std::experimental::reduce(chunk);
		}

		if(const auto processed_values{number_of_chunks*chunk.size()}; processed_values<active_feature_indexes.size())
		{
			transformed[neuron_index]+=std::accumulate(active_feature_indexes.begin()+processed_values, active_feature_indexes.end(), 0, [&](const auto acc, const auto feature_index)
			{
				return acc+neuron[feature_index];
			});
		}
		
		transformed[neuron_index]+=biases[neuron_index];
	}
}
