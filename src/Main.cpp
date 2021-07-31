#include "Game.h"
#include "Platform/Platform.hpp"

int main()
{
	Game game;

	while (!game.Finished())
	{
		game.HandleInput();
		game.Update();
		game.Render();
		game.RestartClock();
	}
}