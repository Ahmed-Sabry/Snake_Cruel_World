#include "Game.h"
#include "MenuState.h"
#include "PlayState.h"
#include "PauseState.h"
#include "GameOverState.h"
#include "LevelSelectState.h"
#include "AchievementState.h"
#include "StatisticsState.h"
#include "SkinSelectState.h"

Game::Game() :
	m_window({ 1366, 768 }, "Hello Cruel World"),
	m_stateManager(m_window, m_audioManager, m_statsManager, m_achievementManager)
{
	m_achievementManager.SetAudioManager(&m_audioManager);

	m_elapsedTime = 0.0f;

	// Load sound effects from files (silently skips missing files)
	static const std::pair<const char*, const char*> soundAssets[] = {
		{ "apple_eat",      "content/audio/sfx/apple_eat.ogg" },
		{ "self_collide",   "content/audio/sfx/self_collide.ogg" },
		{ "wall_death",     "content/audio/sfx/wall_death.ogg" },
		{ "world_shrink",   "content/audio/sfx/world_shrink.ogg" },
		{ "combo_3x",       "content/audio/sfx/combo_3x.ogg" },
		{ "level_complete", "content/audio/sfx/level_complete.ogg" },
		{ "menu_navigate",  "content/audio/sfx/menu_navigate.ogg" },
		{ "menu_select",    "content/audio/sfx/menu_select.ogg" },
		{ "blackout_on",    "content/audio/sfx/blackout_on.ogg" },
		{ "mirror_flip",    "content/audio/sfx/mirror_flip.ogg" },
		{ "apple_miss",     "content/audio/sfx/apple_miss.ogg" },
		{ "achievement_unlock", "content/audio/sfx/achievement_unlock.ogg" },
		{ "skin_select",    "content/audio/sfx/skin_select.ogg" },
		{ "endless_cycle",  "content/audio/sfx/endless_cycle.ogg" },
		{ "endless_warning","content/audio/sfx/endless_warning.ogg" },
	};
	for (const auto& [id, path] : soundAssets)
		m_audioManager.LoadSound(id, path);

	// Fill in any missing sounds with synthesized defaults
	m_audioManager.GenerateDefaultSounds();

	// Register all states
	m_stateManager.RegisterState<MenuState>(StateType::MainMenu);
	m_stateManager.RegisterState<PlayState>(StateType::Gameplay);
	m_stateManager.RegisterState<PauseState>(StateType::Pause);
	m_stateManager.RegisterState<GameOverState>(StateType::GameOver);
	m_stateManager.RegisterState<LevelSelectState>(StateType::LevelSelect);
	m_stateManager.RegisterState<AchievementState>(StateType::Achievements);
	m_stateManager.RegisterState<StatisticsState>(StateType::Statistics);
	m_stateManager.RegisterState<SkinSelectState>(StateType::SkinSelect);

	// Load saved progress
	SaveManager::Load(m_stateManager, m_statsManager, m_achievementManager);

	// Start at main menu
	m_stateManager.SwitchTo(StateType::MainMenu);
}

Game::~Game()
{
	// Save progress on exit
	SaveManager::Save(m_stateManager, m_statsManager, m_achievementManager);
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
