#include "State.h"
#include "Engine.h"
#include "Position.h"

#include <sstream>

using namespace engine;

int main(int argc, char* argv[])
{
	assert(argc >= 3 && "usage: engine <fen> <depth>");
	const std::string_view fen = argv[1];
	const int depth = std::atoi(argv[2]);
	State state{fen};
	if(argc == 4)
	{
		std::istringstream iss{argv[3]};
		std::string move{""};
		while(std::getline(iss, move, ' '))
			state.make(Move{algebraic_to_position(move.substr(0, 2)), algebraic_to_position(move.substr(2, 2))});
	}
	for(;;)
	{
		const Move current_move{generate_move_at_depth(state, depth)};
		std::cout << current_move << "\n";
		state.make(current_move);
		std::string user_move;
		std::cin >> user_move;
		state.make(Move{algebraic_to_position(user_move.substr(0, 2)), algebraic_to_position(user_move.substr(2, 2))});
	}
}
