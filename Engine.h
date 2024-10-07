#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include <array>

#include "Constants.h"
#include "Pieces.h"

namespace engine
{
	class Engine
	{

	public:

		[[nodiscard]] double white_material_value() const;
		[[nodiscard]] double black_material_value() const;
		[[nodiscard]] double evaluate() const;
		void output_weights() const;
		void output_board() const;

	private:

		static std::array<int, 6> piece_values;
		using weightmap_type = std::array<std::array<int, board_size*board_size>, 6>;
		static weightmap_type white_weightmaps;
		static weightmap_type black_weightmaps;

		[[nodiscard]] static weightmap_type generate_black_weightmap();
	};
}

#endif // Engine_h_INCLUDED
