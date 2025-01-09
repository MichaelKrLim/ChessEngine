#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include <array>

#include "Constants.h"
#include "FEN.h"

namespace engine
{
	class Engine
	{
		public:

		[[nodiscard]] double material_value(const Side& side) const;
		[[nodiscard]] double evaluate() const;
		void output_weights() const;

		private:

		static std::array<int, 6> piece_values_;
		using weightmap_type = std::array<std::array<int, board_size*board_size>, 6>;
		static weightmap_type white_weightmaps_;
		static weightmap_type black_weightmaps_;
		static FEN FEN_();

		[[nodiscard]] static weightmap_type generate_black_weightmap();
	};
}

#endif // Engine_h_INCLUDED
