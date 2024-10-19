#ifndef Legal_move_generator_h_INCLUDED
#define Legal_move_generator_h_INCLUDED

#include "Square.h"

#include <array>
#include <vector>

//TODO
namespace engine 
{
	class Legal_move_generator
	{
		struct Magic_square
		{
			const& std::vector<uint64_t> attack_table;
			uint64_t mask;
			uint64_t magic;
			int shift;
		};

		public:

		inline Legal_move_generator() { initialise_attack_table(); };

		private:

		void initialise_attack_table();
		void cast_magic();

		uint64_t reachable_squares();

		std::array<Magic_square, 64> bishop_attack_table_;
		std::array<Magic_square, 64> rook_attack_table_;

		std::vector<uint64_t> attack_table_;

		constexpr std::array<std::array<int, 2>, 8> knight_moves =
		{
        	{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        	{1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    	};
	};
}

#endif // Legal_move_generator_h_INCLUDED
