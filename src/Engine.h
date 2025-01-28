#ifndef Engine_h_INCLUDED
#define Engine_h_INCLUDED

#include "Board.h"
#include "Constants.h"
//#include "Move_generator.h"

#include <array>

namespace engine
{
	class Engine
	{
		public:

		[[nodiscard]] double material_value() const;
		[[nodiscard]] double evaluate() const;
		void output_weights() const;

		private:

		using weightmap_type = std::array<std::array<int, board_size*board_size>, 6>;
		static weightmap_type white_weightmaps_;
		static weightmap_type black_weightmaps_;	
		static std::array<int, 6> piece_values_;
		//const static Move_generator move_generator_{};
		Board board_;

		[[nodiscard]] static weightmap_type generate_black_weightmap();
	};
}

#endif // Engine_h_INCLUDED
