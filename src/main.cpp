#include "Board.h"
#include "Engine.h"
#include "Position.h"

using namespace engine;

int main(int argc, char* argv[])
{
	assert(argc == 3 && "usage: engine <fen> <depth>");
	const std::string_view fen = argv[1];
	const int depth = std::atoi(argv[2]);
	Board board{fen};
	for(;;)
	{
		const Move current_move{generate_move_at_depth(board, depth)};
		std::cout << current_move << "\n";
		board.make(current_move);
		std::string user_move;
		std::cin >> user_move;
		board.make(Move{algebraic_to_position(user_move.substr(0, 2)), algebraic_to_position(user_move.substr(2, 2))});
	}
}
