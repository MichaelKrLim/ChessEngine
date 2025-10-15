#ifndef Neural_network_h_INCLUDED
#define Neural_network_h_INCLUDED

#include <fstream>
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

	[[nodiscard]] double evaluate(const engine::Side side_to_move) const noexcept;
	void refresh_accumulator(std::span<const std::uint16_t> features, const engine::Side side) noexcept;
	void update_accumulator(std::span<const std::uint16_t> removed_features, std::span<const std::uint16_t> added_features, const engine::Side perspective) noexcept;
	[[nodiscard]] inline static std::uint16_t compute_feature_index(const engine::Piece piece
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

	engine::Side_map<std::array<std::int16_t, Feature_transformer::dimensions.neurons>> accumulator{};

	private:

	template <numeric numeric_type, std::size_t size>
	inline std::array<std::int8_t, size> clipped_ReLU(const std::array<numeric_type, size>& input, const int multiple_of_one_value=1) const noexcept
	{
		std::array<std::int8_t, size> output;
		for(auto&& [new_value, value] : std::views::zip(output, input))
			new_value=std::clamp(value/multiple_of_one_value,0,127);
		return output;
	}

	constexpr static Dimensions dense_one_dimensions{512,32},
								dense_two_dimensions{32,32},
								dense_three_dimensions{32,1};

	inline static std::ifstream net_file{"/home/michael/coding/projects/ChessEngine/src/nnue/nn-97f742aaefcd.nnue"};
	inline static NNUE_header header=NNUE_header(net_file);
	inline static Feature_transformer feature_transformer=Feature_transformer(net_file);
	inline static std::uint32_t dense_layers_hash=read_little_endian<decltype(dense_layers_hash)>(net_file);
	inline static Dense_linear_layer<dense_one_dimensions> dense_one=Dense_linear_layer<dense_one_dimensions>(net_file);
	inline static Dense_linear_layer<dense_two_dimensions> dense_two=Dense_linear_layer<dense_two_dimensions>(net_file);
	inline static Dense_linear_layer<dense_three_dimensions> dense_three=Dense_linear_layer<dense_three_dimensions>(net_file);
};

#endif
