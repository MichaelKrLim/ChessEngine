#include "Board.h"
#include "Board_state.h"
#include "Engine.h"
#include "FEN.h"

#include <iostream>

int main()
{
	engine::Engine engine;
	//engine.output_weights();
	std::cout << "\n";
	engine::Board_state board_state(engine::Board(engine::FEN("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R")));
	board_state.output_board();
}
