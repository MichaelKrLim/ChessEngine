#ifndef Dense_linear_layer_h_INCLUDED
#define Dense_linear_layer_h_INCLUDED

#include <experimental/simd>
#include <istream>
#include <vector>

#include "../common.h"
#include "mdspan.h"

template <Dimensions layer_dimensions>
class Dense_linear_layer
{
	public:

	using bias_type=std::int32_t;
	using weight_type=std::int8_t;

	Dense_linear_layer(std::istream& net_file) noexcept;

	[[nodiscard]] std::array<bias_type, layer_dimensions.neurons> transform(const std::array<std::int8_t, layer_dimensions.features>& column_vector) const noexcept;

	private:

	std::vector<bias_type> biases;
	std::vector<weight_type> weights_data;

	template <typename contained_type>
	using helper_weights_container=std::experimental::mdspan<contained_type
												   , std::experimental::extents<std::size_t,layer_dimensions.neurons, layer_dimensions.features>
												   , std::experimental::layout_right>;

	using weights_container=helper_weights_container<weight_type>;
	using const_weights_container=helper_weights_container<const weight_type>;
};

#include "Dense_linear_layer_impl.h"

#endif // Dense_linear_layer_h_INCLUDED
