#ifndef Legal_move_generator_h_INCLUDED
#define Legal_move_generator_h_INCLUDED

#include "Move.h"
#include "Pieces.h"

#include <array>

//TODO
namespace engine 
{
	class Legal_move_generator
	{
		public:

		inline Legal_move_generator() { initialise_masks(); }

		private:

		initialise_masks();
		std::array<uint8_t, 6> masks{};
	};
}

#endif // Legal_move_generator_h_INCLUDED
