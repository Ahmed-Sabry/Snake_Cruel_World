#include "Game.h"
#include "MenuState.h"
#include "PlayState.h"
#include "PauseState.h"
#include "GameOverState.h"
#include "LevelSelectState.h"

Game::Game() :
	m_window({ 1366, 768 }, "Hello Cruel World"),
	m_stateManager(m_window)
{
	m_elapsedTime = 0.0f;

	// Register all states
	m_stateManager.RegisterState<MenuState>(StateType::MainMenu);
	m_stateManager.RegisterState<PlayState>(StateType::Gameplay);
	m_stateManager.RegisterState<PauseState>(StateType::Pause);
	m_stateManager.RegisterState<GameOverState>(StateType::GameOver);
	m_stateManager.RegisterState<LevelSelectState>(StateType::LevelSelect);

	// Load saved progress
	SaveManager::Load(m_stateManager);

	// Start at main menu
	m_stateManager.SwitchTo(StateType::MainMenu);
}

Game::~Game()
{
	// Save progress on exit
	SaveManager::Save(m_stateManager);
}

void Game::Update()
{
	m_window.Update();
	m_stateManager.Update(m_elapsedTime);
}

void Game::HandleInput()
{
	m_stateManager.HandleInput();
}

void Game::Render()
{
	m_stateManager.Render();
}


void Game::RestartClock()
{
	m_elapsedTime = m_clock.restart().asSeconds();
}
