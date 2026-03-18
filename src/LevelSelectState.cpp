#include "LevelSelectState.h"
#include "AudioManager.h"
#include <algorithm>
#include <iostream>

LevelSelectState::LevelSelectState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedLevel(0),
	  m_keyReleased(true),
	  m_animTimer(0.0f)
{
}

void LevelSelectState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "LevelSelectState: Failed to load font" << std::endl;
	m_levels = GetAllLevels();

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();
	window.SetBackground(sf::Color(12, 8, 15));

	// Title
	m_title.setFont(m_font);
	m_title.setString("Choose Your Suffering");
	m_title.setCharacterSize(40);
	m_title.setFillColor(sf::Color(200, 50, 50));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 30.f);

	// Level entries - two columns
	float colWidth = winSize.x / 2.0f;
	float startY = 110.f;
	float rowHeight = 60.f;

	for (int i = 0; i < NUM_LEVELS; i++)
	{
		int col = i / 5;
		int row = i % 5;
		float x = 80.f + col * colWidth;
		float y = startY + row * rowHeight * 2.0f;

		bool unlocked = (i + 1) <= m_stateManager.highestUnlockedLevel;

		// Level name
		m_levelNames[i].setFont(m_font);
		std::string nameStr = std::to_string(i + 1) + ". " + m_levels[i].name;
		if (!unlocked) nameStr += " [LOCKED]";
		m_levelNames[i].setString(nameStr);
		m_levelNames[i].setCharacterSize(22);
		m_levelNames[i].setFillColor(unlocked ? sf::Color(200, 190, 190) : sf::Color(80, 70, 70));
		m_levelNames[i].setPosition(x, y);

		// Subtitle
		m_levelSubtitles[i].setFont(m_font);
		m_levelSubtitles[i].setString(unlocked ? m_levels[i].subtitle : "???");
		m_levelSubtitles[i].setCharacterSize(14);
		m_levelSubtitles[i].setFillColor(sf::Color(130, 110, 110));
		m_levelSubtitles[i].setPosition(x + 20.f, y + 28.f);

		// Stars
		m_levelStars[i].setFont(m_font);
		int stars = m_stateManager.starRatings[i];
		std::string starStr = "";
		for (int s = 0; s < 3; s++)
			starStr += (s < stars) ? "* " : "- ";
		m_levelStars[i].setString(unlocked ? starStr : "");
		m_levelStars[i].setCharacterSize(18);
		m_levelStars[i].setFillColor(sf::Color(255, 215, 0));
		m_levelStars[i].setPosition(x + 20.f, y + 48.f);

		// High score
		m_levelScores[i].setFont(m_font);
		if (unlocked && m_stateManager.highScores[i] > 0)
			m_levelScores[i].setString("Best: " + std::to_string(m_stateManager.highScores[i]));
		else
			m_levelScores[i].setString("");
		m_levelScores[i].setCharacterSize(14);
		m_levelScores[i].setFillColor(sf::Color(150, 140, 140));
		m_levelScores[i].setPosition(x + 20.f, y + 68.f);
	}

	// Back hint
	m_backHint.setFont(m_font);
	m_backHint.setString("ESC - Back to Menu");
	m_backHint.setCharacterSize(16);
	m_backHint.setFillColor(sf::Color(100, 90, 90));
	m_backHint.setPosition(20.f, winSize.y - 40.f);

	m_selectedLevel = std::max(0, std::min(m_stateManager.currentLevel - 1, NUM_LEVELS - 1));
	m_keyReleased = true;
}

void LevelSelectState::OnExit()
{
}

void LevelSelectState::HandleInput()
{
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	bool leftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
	bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	bool escPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);

	if (!upPressed && !downPressed && !leftPressed && !rightPressed && !enterPressed && !escPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;

	if (escPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::MainMenu);
		return;
	}

	if (upPressed)
	{
		m_selectedLevel--;
		if (m_selectedLevel < 0)
			m_selectedLevel = NUM_LEVELS - 1;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (downPressed)
	{
		m_selectedLevel++;
		if (m_selectedLevel >= NUM_LEVELS)
			m_selectedLevel = 0;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (leftPressed && m_selectedLevel >= 5)
	{
		m_selectedLevel -= 5;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (rightPressed && m_selectedLevel < 5)
	{
		m_selectedLevel += 5;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (enterPressed)
	{
		if ((m_selectedLevel + 1) <= m_stateManager.highestUnlockedLevel)
		{
			m_stateManager.GetAudio().PlaySound("menu_select");
			m_stateManager.currentLevel = m_selectedLevel + 1;
			m_stateManager.SwitchTo(StateType::Gameplay);
		}
	}
}

void LevelSelectState::Update(float l_dt)
{
	m_animTimer += l_dt;

	for (int i = 0; i < NUM_LEVELS; i++)
	{
		bool unlocked = (i + 1) <= m_stateManager.highestUnlockedLevel;

		if (i == m_selectedLevel && unlocked)
		{
			float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
			int r = 200 + (int)(55 * pulse);
			m_levelNames[i].setFillColor(sf::Color(r, 80, 60));
		}
		else if (unlocked)
		{
			m_levelNames[i].setFillColor(sf::Color(200, 190, 190));
		}
		else
		{
			m_levelNames[i].setFillColor(sf::Color(80, 70, 70));
		}
	}
}

void LevelSelectState::Render()
{
	Window& window = m_stateManager.GetWindow();

	window.Draw(m_title);

	for (int i = 0; i < NUM_LEVELS; i++)
	{
		window.Draw(m_levelNames[i]);
		window.Draw(m_levelSubtitles[i]);
		window.Draw(m_levelStars[i]);
		window.Draw(m_levelScores[i]);
	}

	window.Draw(m_backHint);
}
