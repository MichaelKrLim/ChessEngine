#ifndef Feature_transformer_h_INCLUDED
#define Feature_transformer_h_INCLUDED

#include <cstdint>
#include <experimental/bits/simd.h>
#include <fstream>
#include <vector>

#include "../common.h"
#include "mdspan.h"

class Feature_transformer
{
	public:

	using bias_type=std::int16_t;
	using weight_type=std::int16_t;


	Feature_transformer(std::ifstream& net_file) noexcept;

	constexpr static Dimensions dimensions{41024,256};
	void transform(std::span<const std::uint16_t> active_feature_indexes
				 , std::span<bias_type, dimensions.neurons> transformed) const noexcept;
	void adjust(std::span<const std::uint16_t> feature_indexes
			  , const auto& reduction
			  , std::span<bias_type, dimensions.neurons> adjusted)
	{
		// Even with column major, it is faster to loop over the features first
		const_weights_container weights{weights_data.data()};
		for(const auto& feature : feature_indexes)
		{
			for(std::size_t i{0}; i<dimensions.neurons; ++i)
				adjusted[i]=reduction(adjusted[i], weights[i,feature]);
		}
	}

	std::uint32_t hash;
	std::vector<bias_type> biases;
	std::vector<weight_type> weights_data;

	private:

	template <typename contained_type>
	using helper_weights_container=std::experimental::mdspan<contained_type
												   , std::experimental::extents<std::size_t, dimensions.neurons, dimensions.features>
												   , std::experimental::layout_left>;

	using weights_container=helper_weights_container<weight_type>;
	using const_weights_container=helper_weights_container<const weight_type>;
};

#endif // Feature_transformer_h_INCLUDED
