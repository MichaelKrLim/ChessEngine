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

	[[nodiscard]] std::vector<bias_type> transform(const std::vector<std::int8_t>& column_vector) const noexcept;

	private:

	std::vector<bias_type> biases;
	std::vector<std::vector<weight_type>> weights;
};

#include "Dense_linear_layer_impl.h"

#endif // Dense_linear_layer_h_INCLUDED
