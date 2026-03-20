#include "GameOverState.h"
#include "AudioManager.h"
#include "LevelConfig.h"
#include "SaveManager.h"
#include "StatsManager.h"
#include "AchievementManager.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <vector>

const char* GameOverState::s_deathTaunts[] = {
	"That was almost impressive.",
	"The walls send their regards.",
	"You are your own worst enemy. Literally.",
	"The call is coming from inside the snake.",
	"Better luck next time. Or not.",
	"The world barely had to try.",
	"Have you considered a different hobby?",
	"The snake deserved better.",
	"So close. And yet so far.",
	"Error 404: Skill not found.",
	"The game thanks you for the entertainment.",
	"That was educational. For us, not you.",
	"Gravity works differently when you're a snake.",
	"The walls barely noticed you.",
	"Achievement unlocked: Spectacular Failure."
};
const int GameOverState::s_deathTauntCount = 15;

const char* GameOverState::s_victoryTaunts[] = {
	"Impressive. The world will try harder next time.",
	"You survived. The world is disappointed.",
	"Don't let this go to your head.",
	"Victory. For now.",
	"The next level heard about this. It's angry.",
	"You've earned a brief moment of peace.",
	"Well done. Now do it with three stars.",
	"The cruel world grudgingly respects you."
};
const int GameOverState::s_victoryTauntCount = 8;

const char* GameOverState::s_fastDeathTaunts[] = {
	"That was... efficient.",
	"Speed run?",
	"Blink and you missed it. So did we.",
	"New personal worst!",
	"That might be a record. Not a good one."
};
const int GameOverState::s_fastDeathTauntCount = 5;

GameOverState::GameOverState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedItem(0),
	  m_itemCount(3),
	  m_hasNextLevel(false),
	  m_keyReleased(false)
{
}

void GameOverState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "GameOverState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();
	// Derive paper tone from the completed level
	int idx = m_stateManager.currentLevel - 1;
	const auto& levels = GetAllLevels();
	sf::Color paperBg = (idx >= 0 && idx < NUM_LEVELS)
		? levels[idx].paperTone : sf::Color(235, 225, 210);
	window.SetBackground(paperBg);

	bool won = m_stateManager.levelComplete;
	bool endless = m_stateManager.endlessMode;

	// Title
	m_title.setFont(m_font);
	if (endless)
		m_title.setString("Game Over");
	else
		m_title.setString(won ? "Level Complete!" : "You Died.");
	m_title.setCharacterSize(48);
	m_title.setFillColor(won ? sf::Color(45, 110, 55) : sf::Color(170, 55, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 80.f);

	// Taunt
	const char* taunt;
	if (won)
	{
		taunt = s_victoryTaunts[rand() % s_victoryTauntCount];
	}
	else if (m_stateManager.levelTime < 5.0f)
	{
		taunt = s_fastDeathTaunts[rand() % s_fastDeathTauntCount];
	}
	else
	{
		taunt = s_deathTaunts[rand() % s_deathTauntCount];
	}

	m_taunt.setFont(m_font);
	m_taunt.setString(taunt);
	m_taunt.setCharacterSize(20);
	sf::Color goInk = (idx >= 0 && idx < NUM_LEVELS)
		? levels[idx].inkTint : sf::Color(60, 50, 45);
	sf::Color goLightInk(std::min(255, (int)goInk.r + 60),
						  std::min(255, (int)goInk.g + 50),
						  std::min(255, (int)goInk.b + 45));
	m_taunt.setFillColor(goLightInk);
	sf::FloatRect tauntBounds = m_taunt.getLocalBounds();
	m_taunt.setPosition((winSize.x - tauntBounds.width) / 2.0f, 150.f);

	// Stats
	std::string statLabels[] = {
		"Score: " + std::to_string(m_stateManager.score),
		"Apples: " + std::to_string(m_stateManager.applesEaten),
		"Max Combo: x" + std::to_string(m_stateManager.combo),
		"",
		"Self Collisions: " + std::to_string(m_stateManager.selfCollisions)
	};

	// Time formatting
	int mins = (int)m_stateManager.levelTime / 60;
	int secs = (int)m_stateManager.levelTime % 60;
	std::ostringstream ts;
	ts << "Time: " << mins << ":" << std::setw(2) << std::setfill('0') << secs;
	statLabels[3] = ts.str();

	float statsStartY = 220.f;
	for (int i = 0; i < 5; i++)
	{
		m_stats[i].setFont(m_font);
		m_stats[i].setString(statLabels[i]);
		m_stats[i].setCharacterSize(20);
		m_stats[i].setFillColor(goInk);
		sf::FloatRect bounds = m_stats[i].getLocalBounds();
		m_stats[i].setPosition((winSize.x - bounds.width) / 2.0f, statsStartY + i * 35.f);
	}

	// Star rating display for wins
	if (won)
	{
		int stars = 1;
		if (idx >= 0 && idx < NUM_LEVELS)
		{
			if (m_stateManager.selfCollisions <= levels[idx].starThreshold2)
				stars = 2;
			if (m_stateManager.selfCollisions <= levels[idx].starThreshold3)
				stars = 3;
		}
		std::string starStr = "";
		for (int i = 0; i < 3; i++)
			starStr += (i < stars) ? "* " : "- ";
		m_stats[4].setString(starStr);
		m_stats[4].setFillColor(sf::Color(200, 170, 30)); // Gold ink
		sf::FloatRect bounds = m_stats[4].getLocalBounds();
		m_stats[4].setPosition((winSize.x - bounds.width) / 2.0f, statsStartY + 4 * 35.f);
	}

	// Menu items — "Next Level" only on victory and if not the last level (and not endless)
	m_hasNextLevel = won && !endless && m_stateManager.currentLevel < NUM_LEVELS;

	std::vector<std::string> items;
	m_menuActions.clear();
	if (m_hasNextLevel)
	{
		items.push_back("Next Level");
		m_menuActions.push_back(GameOverAction::NextLevel);
	}
	items.push_back("Retry");
	m_menuActions.push_back(GameOverAction::Retry);
	if (!endless)
	{
		items.push_back("Level Select");
		m_menuActions.push_back(GameOverAction::LevelSelect);
	}
	items.push_back("Main Menu");
	m_menuActions.push_back(GameOverAction::MainMenu);
	m_itemCount = (int)items.size();

	// Save progress
	SaveManager::Save(m_stateManager, m_stateManager.GetStats(),
					  m_stateManager.GetAchievements());

	float menuStartY = 460.f;
	float spacing = 50.f;

	for (int i = 0; i < m_itemCount; i++)
	{
		m_menuItems[i].setFont(m_font);
		m_menuItems[i].setString(items[i]);
		m_menuItems[i].setCharacterSize(28);
		sf::FloatRect bounds = m_menuItems[i].getLocalBounds();
		m_menuItems[i].setPosition((winSize.x - bounds.width) / 2.0f, menuStartY + i * spacing);
	}

	m_selectedItem = 0;
	m_keyReleased = false;

	// Screen shake on death
	if (!won)
		m_screenShake.Trigger(0.4f, 4.0f);
}

void GameOverState::OnExit()
{
	m_screenShake.Reset(m_stateManager.GetWindow());
}

void GameOverState::HandleInput()
{
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	bool rPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::R);

	if (!upPressed && !downPressed && !enterPressed && !rPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;

	// Quick restart
	if (rPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::Gameplay);
		return;
	}

	if (upPressed)
	{
		m_selectedItem--;
		if (m_selectedItem < 0)
			m_selectedItem = m_itemCount - 1;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (downPressed)
	{
		m_selectedItem++;
		if (m_selectedItem >= m_itemCount)
			m_selectedItem = 0;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (enterPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");

		if (m_selectedItem >= 0 && m_selectedItem < (int)m_menuActions.size())
		{
			switch (m_menuActions[m_selectedItem])
			{
				case GameOverAction::NextLevel:
					m_stateManager.currentLevel++;
					m_stateManager.SwitchTo(StateType::Gameplay);
					break;
				case GameOverAction::Retry:
					m_stateManager.SwitchTo(StateType::Gameplay);
					break;
				case GameOverAction::LevelSelect:
					m_stateManager.endlessMode = false;
					m_stateManager.SwitchTo(StateType::LevelSelect);
					break;
				case GameOverAction::MainMenu:
					m_stateManager.endlessMode = false;
					m_stateManager.SwitchTo(StateType::MainMenu);
					break;
				default:
					break;
			}
		}
	}
}

void GameOverState::Update(float l_dt)
{
	// Screen shake
	m_screenShake.Update(l_dt, m_stateManager.GetWindow());

	// Menu item colors
	for (int i = 0; i < m_itemCount; i++)
	{
		if (i == m_selectedItem)
			m_menuItems[i].setFillColor(sf::Color(170, 55, 40));
		else
			m_menuItems[i].setFillColor(sf::Color(100, 90, 80));
	}
}

void GameOverState::Render()
{
	Window& window = m_stateManager.GetWindow();

	window.Draw(m_title);
	window.Draw(m_taunt);

	for (int i = 0; i < 5; i++)
		window.Draw(m_stats[i]);

	for (int i = 0; i < m_itemCount; i++)
		window.Draw(m_menuItems[i]);
}
