#ifndef Feature_transformer_h_INCLUDED
#define Feature_transformer_h_INCLUDED

#include <fstream>
#include <vector>

#include "../common.h"
#include "../../mdspan.h"

class Feature_transformer
{
	using bias_type=std::int16_t;
	using weight_type=std::int16_t;

	public:

	Feature_transformer(std::ifstream& net_file) noexcept;

	constexpr static Dimensions dimensions{41024,256};
	void transform(const std::vector<std::uint16_t>& active_feature_indexes
												 , std::span<bias_type, dimensions.neurons> transformed) const noexcept;
	auto weights_view() const noexcept
	{
		return const_weights_container{data.data()};
	}

	std::uint32_t hash;
	std::vector<bias_type> biases;
	std::vector<weight_type> data;

	private:

	template <typename contained_type>
	using helper_weights_container=std::experimental::mdspan<contained_type
												   , std::experimental::extents<std::size_t, dimensions.neurons, dimensions.features>
												   , std::experimental::layout_left>;

	using weights_container=helper_weights_container<weight_type>;
	using const_weights_container=helper_weights_container<const weight_type>;
};

#endif // Feature_transformer_h_INCLUDED
