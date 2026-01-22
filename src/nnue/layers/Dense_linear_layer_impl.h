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
		// this is a transform reduce, HOWEVER this is implemented by reducing unrolled chunks of 4 at a time which is less performant in our case
		const auto neuron{std::submdspan(weights, neuron_index, std::full_extent)};
		std::int32_t sum{biases[neuron_index]};
		for(std::size_t feature_index{0}; const auto& feature : column_vector)
			sum+=neuron[feature_index++]*feature;
		result[neuron_index]=sum;
	}
	return result;
}
