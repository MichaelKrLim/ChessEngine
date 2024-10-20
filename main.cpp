#include "Board.h"
#include "Engine.h"
#include "FEN.h"

#include <iostream>

// opening posiiton - "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R"
int main()
{
	engine::Engine engine;
	//engine.output_weights();
	std::cout << "\n";
	engine::Board board(engine::FEN("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R"));
	board.output();
}
