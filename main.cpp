#include "Board.h"
#include "Engine.h"
#include "FEN.h"

#include <iostream>

using namespace engine;

// opening position - "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R"
int main()
{
	Engine engine;
	//engine.output_weights();
	std::cout << "\n";
	Board board("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R", engine::FEN());
	std::cout << board.white.pieces[0].pretty_string();
}
