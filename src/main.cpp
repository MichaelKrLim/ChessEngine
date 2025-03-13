#include "Board.h"
#include "Move_generator.h"

using namespace engine;

int main()
{
	Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	legal_moves(board);
}
