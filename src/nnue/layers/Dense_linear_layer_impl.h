template <Dimensions layer_dimensions>
Dense_linear_layer<layer_dimensions>::Dense_linear_layer(std::ifstream& net_file) noexcept
{
	biases.resize(layer_dimensions.neurons);
	weights.resize(layer_dimensions.neurons, std::vector<weight_type>(layer_dimensions.features));

	for(auto& bias : biases)
		bias=read_little_endian<bias_type>(net_file);

	for(auto& row : weights)
	{
		for(auto& weight : row)
			weight=read_little_endian<weight_type>(net_file);
	}
}

template <Dimensions layer_dimensions>
std::vector<typename Dense_linear_layer<layer_dimensions>::bias_type> Dense_linear_layer<layer_dimensions>::transform(const std::vector<std::int8_t>& column_vector) const noexcept
{
	std::vector<bias_type> result(layer_dimensions.neurons);
	for(std::size_t neuron_index{0}; neuron_index<layer_dimensions.neurons; ++neuron_index)
	{
		const auto& neuron{weights[neuron_index]};
		for(std::size_t i{0}; i<column_vector.size(); ++i)
		{
			result[neuron_index]+=column_vector[i]*neuron[i];
		}
		result[neuron_index]+=biases[neuron_index];
	}
	return result;
}
