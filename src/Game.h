#pragma once

#include "Window.h"
#include "Snake.h"
#include "World.h"


class Game
{
public:
	Game();
	~Game();

	void Update();
	void HandleInput();
	void Render();
	void RestartClock();

	inline bool Finished() { return m_window.IsDone(); }

private:
	Window m_window;
	Snake m_snake;
	World m_world;

	bool e = false; // temp
	sf::Clock m_clock;
	float m_elapsedTime;
};