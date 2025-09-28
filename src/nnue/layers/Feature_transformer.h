#ifndef Feature_transformer_h_INCLUDED
#define Feature_transformer_h_INCLUDED

#include <fstream>
#include <vector>

#include "../common.h"

class Feature_transformer
{
	using bias_type=std::int16_t;
	using weight_type=std::int16_t;

	public:

	Feature_transformer(std::ifstream& net_file) noexcept;

	[[nodiscard]] std::vector<bias_type> transform(const std::vector<std::int16_t>& active_feature_indexes) const noexcept;

	private:

	constexpr static Dimensions dimensions{41024,256};

	std::uint32_t hash;
	std::vector<bias_type> biases;
	std::vector<std::vector<weight_type>> weights;
};

#endif // Feature_transformer_h_INCLUDED
