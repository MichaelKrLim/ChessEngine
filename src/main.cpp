#include "Board.h"
#include "Engine.h"
#include "Position.h"
#include "Uci_handler.h"

#include <sstream>

using namespace engine;

int main()
{
	Uci_handler uci_handler;
	uci_handler.start_listening();
}
