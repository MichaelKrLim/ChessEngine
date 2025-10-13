#ifndef Dense_linear_layer_h_INCLUDED
#define Dense_linear_layer_h_INCLUDED

#include <fstream>
#include <vector>

#include "../common.h"

template <Dimensions layer_dimensions>
class Dense_linear_layer
{
	using bias_type=std::int32_t;
	using weight_type=std::int8_t;

	public:

	Dense_linear_layer(std::ifstream& net_file) noexcept;

	[[nodiscard]] std::array<bias_type, layer_dimensions.neurons> transform(const std::array<std::int8_t, layer_dimensions.features>& column_vector) const noexcept;

	private:

	std::vector<bias_type> biases;
	std::vector<std::vector<weight_type>> weights;
};

#include "Dense_linear_layer_impl.h"

#endif // Dense_linear_layer_h_INCLUDED
