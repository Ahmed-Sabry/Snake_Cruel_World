#pragma once

#include "Window.h"
#include "AudioManager.h"
#include "StateManager.h"
#include "SaveManager.h"

class Game
{
public:
	Game();
	~Game();

	void Update();
	void HandleInput();
	void Render();
	void RestartClock();

	inline bool Finished()
	{
		return m_window.IsDone();
	}

private:
	Window m_window;
	AudioManager m_audioManager;
	StateManager m_stateManager;

	sf::Clock m_clock;
	float m_elapsedTime;
};
