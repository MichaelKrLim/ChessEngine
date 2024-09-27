#include <iostream>

#include "Board_state.h"

void init_weightmaps(Board_state& board_state)
{
	board_state.white_weightmaps() = 
	board_state.piece_values() =

	board_state.black_weightmaps() = std::ranges::reverse(board_state.white_weightmaps);
}

int main()
{
    Board_state board_state;
    
}
