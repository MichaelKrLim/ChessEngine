#include "Board_state.h"
#include "Constants.h"

#include <algorithm>
#include <iostream>

#include <cctype>

using namespace engine;

void Board_state::output_board() const
{
	board_.output();
}

bool Board_state::white_is_mated() const
{
	//TODO
}

bool Board_state::black_is_mated() const
{
	//TODO
}

bool Board_state::is_draw() const
{
	//TODO
}
