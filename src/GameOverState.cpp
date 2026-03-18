#include "GameOverState.h"
#include "LevelConfig.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>

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
	  m_keyReleased(false),
	  m_shakeTimer(0.0f),
	  m_shakeOffsetX(0.0f),
	  m_shakeOffsetY(0.0f)
{
}

void GameOverState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "GameOverState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();
	window.SetBackground(sf::Color(10, 5, 8));

	bool won = m_stateManager.levelComplete;

	// Title
	m_title.setFont(m_font);
	m_title.setString(won ? "Level Complete!" : "You Died.");
	m_title.setCharacterSize(48);
	m_title.setFillColor(won ? sf::Color(100, 255, 100) : sf::Color(200, 50, 50));
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
	m_taunt.setFillColor(sf::Color(180, 140, 140));
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
		m_stats[i].setFillColor(sf::Color(200, 190, 190));
		sf::FloatRect bounds = m_stats[i].getLocalBounds();
		m_stats[i].setPosition((winSize.x - bounds.width) / 2.0f, statsStartY + i * 35.f);
	}

	// Star rating display for wins
	if (won)
	{
		int stars = 1;
		auto levels = GetAllLevels();
		int idx = m_stateManager.currentLevel - 1;
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
		m_stats[4].setFillColor(sf::Color(255, 215, 0));
		sf::FloatRect bounds = m_stats[4].getLocalBounds();
		m_stats[4].setPosition((winSize.x - bounds.width) / 2.0f, statsStartY + 4 * 35.f);
	}

	// Menu items
	std::string items[] = { "Retry", "Level Select", "Main Menu" };
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
		m_shakeTimer = 0.4f;
}

void GameOverState::OnExit()
{
	// Reset view
	Window& window = m_stateManager.GetWindow();
	window.SetView(window.GetDefaultView());
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
		m_stateManager.SwitchTo(StateType::Gameplay);
		return;
	}

	if (upPressed)
	{
		m_selectedItem--;
		if (m_selectedItem < 0)
			m_selectedItem = m_itemCount - 1;
	}
	else if (downPressed)
	{
		m_selectedItem++;
		if (m_selectedItem >= m_itemCount)
			m_selectedItem = 0;
	}
	else if (enterPressed)
	{
		switch (m_selectedItem)
		{
			case 0: // Retry
				m_stateManager.SwitchTo(StateType::Gameplay);
				break;
			case 1: // Level Select
				m_stateManager.SwitchTo(StateType::LevelSelect);
				break;
			case 2: // Main Menu
				m_stateManager.SwitchTo(StateType::MainMenu);
				break;
			default:
				break;
		}
	}
}

void GameOverState::Update(float l_dt)
{
	// Screen shake
	if (m_shakeTimer > 0.0f)
	{
		m_shakeTimer -= l_dt;
		float intensity = m_shakeTimer * 10.0f;
		m_shakeOffsetX = ((rand() % 100) / 100.0f - 0.5f) * intensity;
		m_shakeOffsetY = ((rand() % 100) / 100.0f - 0.5f) * intensity;

		sf::View view = m_stateManager.GetWindow().GetDefaultView();
		view.move(m_shakeOffsetX, m_shakeOffsetY);
		m_stateManager.GetWindow().SetView(view);
	}
	else
	{
		m_stateManager.GetWindow().SetView(m_stateManager.GetWindow().GetDefaultView());
	}

	// Menu item colors
	for (int i = 0; i < m_itemCount; i++)
	{
		if (i == m_selectedItem)
			m_menuItems[i].setFillColor(sf::Color(255, 100, 80));
		else
			m_menuItems[i].setFillColor(sf::Color(180, 170, 170));
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
