#include <fstream>
#include <ranges>

#include "common.h"
#include "layers/Dense_linear_layer.h"
#include "layers/Feature_transformer.h"

class Neural_network
{
	public:

	Neural_network(std::ifstream& net_file)
		: feature_transformer(net_file)
		, dense_layers_hash(read_little_endian<decltype(dense_layers_hash)>(net_file))
		, dense_one(net_file)
		, dense_two(net_file)
		, dense_three(net_file)
	{};

	[[nodiscard]] std::int32_t evaluate(const std::vector<std::int16_t>& white_active_feature_indexes, const std::vector<std::int16_t>& black_active_feature_indexes) const noexcept;

	private:

	template <numeric numeric_type>
	inline std::vector<std::int8_t> clipped_ReLU(const std::vector<numeric_type>& input) const noexcept
	{
		std::vector<std::int8_t> result(input.size());
		for(auto&& [new_value, value] : std::views::zip(result, input))
		{
			new_value=std::clamp(value, numeric_type{0}, numeric_type{1});
		}
		return result;
	}

	constexpr static Dimensions dense_one_dimensions{512,32},
								dense_two_dimensions{32,32},
								dense_three_dimensions{32,1};

	Feature_transformer feature_transformer;
	std::uint32_t dense_layers_hash;
	Dense_linear_layer<dense_one_dimensions> dense_one;
	Dense_linear_layer<dense_two_dimensions> dense_two;
	Dense_linear_layer<dense_three_dimensions> dense_three;
};
