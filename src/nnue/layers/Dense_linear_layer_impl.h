#include <execution>
#include <numeric>

template <Dimensions layer_dimensions>
Dense_linear_layer<layer_dimensions>::Dense_linear_layer(std::istream& net_file) noexcept
	: biases(layer_dimensions.neurons)
	, weights_data(layer_dimensions.neurons*layer_dimensions.features)
{
	for(auto& bias : biases)
		bias=read_little_endian<bias_type>(net_file);

	weights_container weights{weights_data.data()};
	for(std::size_t neuron_index{0}; neuron_index<layer_dimensions.neurons; ++neuron_index)
	{
		for(std::size_t feature_index{0}; feature_index<layer_dimensions.features; ++feature_index)
			weights[neuron_index, feature_index]=read_little_endian<weight_type>(net_file);
	}
}

template <Dimensions layer_dimensions>
std::array<typename Dense_linear_layer<layer_dimensions>::bias_type, layer_dimensions.neurons> Dense_linear_layer<layer_dimensions>::transform(const std::array<std::uint8_t, layer_dimensions.features>& column_vector) const noexcept
{
	std::array<bias_type, layer_dimensions.neurons> result{};
	const_weights_container weights{weights_data.data()};
	for(std::size_t neuron_index{0}; neuron_index<layer_dimensions.neurons; ++neuron_index)
	{
		// this prevents the compiler from swapping our loops around which causes slower single byte loads
		std::atomic_signal_fence(std::memory_order_seq_cst);

		std::int32_t accumulator{biases[neuron_index]};
		const std::int8_t* const neuron{&weights[neuron_index,0]};
		// the casts here allow us to use 'vpmaddwd'
		for(std::size_t feature_index{0}; feature_index<column_vector.size(); ++feature_index)
			accumulator+=static_cast<std::int16_t>(neuron[feature_index])*static_cast<std::int16_t>(column_vector[feature_index]);
		result[neuron_index]=accumulator;
	}
	return result;
}
