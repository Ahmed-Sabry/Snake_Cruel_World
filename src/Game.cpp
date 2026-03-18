#include "Game.h"
#include "MenuState.h"
#include "PlayState.h"
#include "PauseState.h"
#include "GameOverState.h"
#include "LevelSelectState.h"

Game::Game() :
	m_window({ 1366, 768 }, "Hello Cruel World"),
	m_stateManager(m_window, m_audioManager)
{
	m_elapsedTime = 0.0f;

	// Load sound effects from files (silently skips missing files)
	m_audioManager.LoadSound("apple_eat", "content/audio/sfx/apple_eat.ogg");
	m_audioManager.LoadSound("self_collide", "content/audio/sfx/self_collide.ogg");
	m_audioManager.LoadSound("wall_death", "content/audio/sfx/wall_death.ogg");
	m_audioManager.LoadSound("world_shrink", "content/audio/sfx/world_shrink.ogg");
	m_audioManager.LoadSound("combo_3x", "content/audio/sfx/combo_3x.ogg");
	m_audioManager.LoadSound("level_complete", "content/audio/sfx/level_complete.ogg");
	m_audioManager.LoadSound("menu_navigate", "content/audio/sfx/menu_navigate.ogg");
	m_audioManager.LoadSound("menu_select", "content/audio/sfx/menu_select.ogg");

	// Fill in any missing sounds with synthesized defaults
	m_audioManager.GenerateDefaultSounds();

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
