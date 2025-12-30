#ifndef Neural_network_h_INCLUDED
#define Neural_network_h_INCLUDED

#include <filesystem>
#include <istream>
#include <ranges>

#include "common.h"
#include "../Constants.h"
#include "layers/Dense_linear_layer.h"
#include "layers/Feature_transformer.h"
#include "NNUE_header.h"
#include "../Pieces.h"
#include "../Position.h"

class Neural_network
{
	public:

	Neural_network(std::istream& is);
	Neural_network(const std::filesystem::path& path);
	[[nodiscard]] static Neural_network load_from_file(const std::filesystem::path& path) noexcept;
	[[nodiscard]] int evaluate(const engine::Side side_to_move, const engine::Side_map<std::array<std::int16_t, Feature_transformer::dimensions.neurons>>& accumulator) const noexcept;
	void transform_features(std::span<const std::uint16_t> features, std::span<Feature_transformer::bias_type, Feature_transformer::dimensions.neurons> accumulator) const noexcept
	{
		feature_transformer.transform(features,accumulator);
	}
	void adjust_accumulator(std::span<const std::uint16_t> feature_indices, const auto& reduction, std::span<Feature_transformer::bias_type, Feature_transformer::dimensions.neurons> adjusted) const noexcept
	{
		feature_transformer.adjust(feature_indices,reduction,adjusted);
	}
	[[nodiscard]] static std::uint16_t compute_feature_index(const engine::Piece piece
											 , const engine::Position& position
											 , const engine::Position& king_square
											 , const engine::Side current_side
											 , const engine::Side perspective) noexcept
	{
		const auto feature_index=[&](const auto& index_strategy, const engine::Side current_side)
		{
			const auto piece_index_no_king{std::to_underlying(piece)-1};
			const auto piece_feature_index{2*piece_index_no_king + std::to_underlying(current_side)};
			return 1 + index_strategy(position) + index_strategy(king_square) + (piece_feature_index+10*index_strategy(king_square))*64;
		};

		if(perspective==engine::Side::white)
			return feature_index(engine::to_index, current_side);
		if(perspective==engine::Side::black)
			return feature_index([](const engine::Position& position){ return 63-to_index(position); }, other_side(current_side));
		std::unreachable();
	};

	private:

	template <numeric numeric_type, std::size_t size>
	std::array<std::int8_t, size> clipped_ReLU(const std::array<numeric_type, size>& input, const int multiple_of_one_value=1) const noexcept
	{
		std::array<std::int8_t, size> output;
		for(auto&& [new_value, value] : std::views::zip(output, input))
			new_value=std::clamp(value/multiple_of_one_value,0,127);
		return output;
	}

	constexpr static Dimensions dense_one_dimensions{512,32},
								dense_two_dimensions{32,32},
								dense_three_dimensions{32,1};

	NNUE_header header;
	Feature_transformer feature_transformer;
	std::uint32_t dense_layers_hash;
	Dense_linear_layer<dense_one_dimensions> dense_one;
	Dense_linear_layer<dense_two_dimensions> dense_two;
	Dense_linear_layer<dense_three_dimensions> dense_three;
};

#endif
