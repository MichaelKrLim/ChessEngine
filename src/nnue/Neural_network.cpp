#include "Neural_network.h"

double Neural_network::evaluate(const std::vector<std::int16_t>& white_active_feature_indexes, const std::vector<std::int16_t>& black_active_feature_indexes) const noexcept
{
	std::vector transformed_features(feature_transformer.transform(white_active_feature_indexes));
	std::ranges::copy(feature_transformer.transform(black_active_feature_indexes), std::back_inserter(transformed_features));

	const auto d1_transformed{dense_one.transform(clipped_ReLU(transformed_features))};
	const auto d2_transformed{dense_two.transform(clipped_ReLU(d1_transformed,64))};
	const auto d3_transformed{dense_three.transform(clipped_ReLU(d2_transformed,64))};

	return d3_transformed.front()/(600.0*16.0);
}
