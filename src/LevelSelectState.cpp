#include "LevelSelectState.h"
#include "AudioManager.h"
#include "AchievementManager.h"
#include "InkRenderer.h"
#include <algorithm>
#include <iostream>

LevelSelectState::LevelSelectState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedLevel(0),
	  m_keyReleased(true),
	  m_animTimer(0.0f),
	  m_cheatTextTimer(0.0f)
{
}

void LevelSelectState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "LevelSelectState: Failed to load font" << std::endl;
	m_levels = GetAllLevels();

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();

	// Paper background
	LevelConfig selectConfig{};
	selectConfig.id = 0;
	selectConfig.paperTone = sf::Color(240, 232, 218);
	selectConfig.inkTint = sf::Color(50, 40, 35);
	selectConfig.corruption = 0.05f;
	m_paperBg.Generate(selectConfig, winSize.x, winSize.y);
	window.SetBackground(selectConfig.paperTone);

	// Title
	m_title.setFont(m_font);
	m_title.setString("Choose Your Suffering");
	m_title.setCharacterSize(40);
	m_title.setFillColor(sf::Color(180, 50, 40));
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
		m_levelNames[i].setFillColor(unlocked ? sf::Color(60, 50, 45) : sf::Color(170, 160, 155));
		m_levelNames[i].setPosition(x, y);

		// Subtitle
		m_levelSubtitles[i].setFont(m_font);
		m_levelSubtitles[i].setString(unlocked ? m_levels[i].subtitle : "???");
		m_levelSubtitles[i].setCharacterSize(14);
		m_levelSubtitles[i].setFillColor(sf::Color(120, 100, 90));
		m_levelSubtitles[i].setPosition(x + 20.f, y + 28.f);

		// Stars
		m_levelStars[i].setFont(m_font);
		int stars = m_stateManager.starRatings[i];
		std::string starStr = "";
		for (int s = 0; s < 3; s++)
			starStr += (s < stars) ? "* " : "- ";
		m_levelStars[i].setString(unlocked ? starStr : "");
		m_levelStars[i].setCharacterSize(18);
		m_levelStars[i].setFillColor(sf::Color(200, 170, 30));
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
	m_keyReleased = false; // require key release before accepting input

	// Konami code cheat
	m_konamiProgress.clear();
	m_cheatTextTimer = 0.0f;
	m_cheatText.setFont(m_font);
	m_cheatText.setString("The world gives in... for now.");
	m_cheatText.setCharacterSize(28);
	sf::FloatRect ctBounds = m_cheatText.getLocalBounds();
	m_cheatText.setOrigin(ctBounds.left + ctBounds.width / 2.0f,
						  ctBounds.top + ctBounds.height / 2.0f);
	m_cheatText.setPosition(winSize.x / 2.0f, winSize.y / 2.0f);
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

	// Track arrow keys for Konami code (Up Up Down Down Left Right Left Right)
	sf::Keyboard::Key konamiKey = sf::Keyboard::Unknown;
	if (upPressed && !sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		konamiKey = sf::Keyboard::Up;
	else if (downPressed && !sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		konamiKey = sf::Keyboard::Down;
	else if (leftPressed && !sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		konamiKey = sf::Keyboard::Left;
	else if (rightPressed && !sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		konamiKey = sf::Keyboard::Right;

	if (konamiKey != sf::Keyboard::Unknown)
	{
		m_konamiProgress.push_back(konamiKey);
		if (m_konamiProgress.size() > 8)
			m_konamiProgress.erase(m_konamiProgress.begin());

		static const sf::Keyboard::Key konamiCode[] = {
			sf::Keyboard::Up, sf::Keyboard::Up,
			sf::Keyboard::Down, sf::Keyboard::Down,
			sf::Keyboard::Left, sf::Keyboard::Right,
			sf::Keyboard::Left, sf::Keyboard::Right
		};

		if (m_konamiProgress.size() == 8 &&
			std::equal(m_konamiProgress.begin(), m_konamiProgress.end(), konamiCode))
		{
			m_stateManager.highestUnlockedLevel = NUM_LEVELS;
			m_stateManager.GetAudio().PlaySound("level_complete");
			m_stateManager.GetAchievements().OnKonamiCode();
			m_konamiProgress.clear();
			OnEnter(); // refresh UI to show unlocked levels
			m_cheatTextTimer = 3.0f; // set after OnEnter (which resets it to 0)
			return;
		}
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

	if (m_cheatTextTimer > 0.0f)
		m_cheatTextTimer -= l_dt;

	for (int i = 0; i < NUM_LEVELS; i++)
	{
		bool unlocked = (i + 1) <= m_stateManager.highestUnlockedLevel;

		if (i == m_selectedLevel && unlocked)
		{
			float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
			int r = 140 + (int)(60 * pulse);
			m_levelNames[i].setFillColor(sf::Color(r, 40, 30));
		}
		else if (unlocked)
		{
			m_levelNames[i].setFillColor(sf::Color(60, 50, 45));
		}
		else
		{
			m_levelNames[i].setFillColor(sf::Color(170, 160, 155));
		}
	}
}

void LevelSelectState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	// Paper background
	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	// Title with wobbly underline
	window.Draw(m_title);
	sf::FloatRect titleBounds = m_title.getGlobalBounds();
	InkRenderer::DrawWobblyLine(target,
								titleBounds.left, titleBounds.top + titleBounds.height + 5,
								titleBounds.left + titleBounds.width, titleBounds.top + titleBounds.height + 5,
								sf::Color(180, 50, 40, 120), 1.5f, 0.1f, 777);

	for (int i = 0; i < NUM_LEVELS; i++)
	{
		bool unlocked = (i + 1) <= m_stateManager.highestUnlockedLevel;

		// Draw a wobbly box around each level entry
		sf::FloatRect nameBounds = m_levelNames[i].getGlobalBounds();
		float boxX = nameBounds.left - 10;
		float boxY = nameBounds.top - 5;
		float boxW = 280;
		float boxH = 90;

		sf::Color boxColor = unlocked ? sf::Color(60, 50, 45, 40) : sf::Color(150, 140, 130, 25);
		InkRenderer::DrawWobblyRect(target, boxX, boxY, boxW, boxH,
									sf::Color::Transparent, boxColor, 1.0f,
									unlocked ? 0.1f : 0.05f, (unsigned int)(i * 31));

		// Selection indicator
		if (i == m_selectedLevel)
		{
			InkRenderer::DrawWobblyRect(target, boxX - 3, boxY - 3, boxW + 6, boxH + 6,
										sf::Color::Transparent, sf::Color(180, 50, 40, 100), 1.5f,
										0.25f, (unsigned int)(m_animTimer * 8.0f));
		}

		window.Draw(m_levelNames[i]);
		window.Draw(m_levelSubtitles[i]);

		// Hand-drawn stars instead of text stars
		int stars = m_stateManager.starRatings[i];
		if (unlocked)
		{
			for (int s = 0; s < 3; s++)
			{
				float sx = nameBounds.left + 20.0f + s * 18.0f;
				float sy = nameBounds.top + 50.0f;
				bool filled = s < stars;
				InkRenderer::DrawStar(target, sx, sy, 7.0f,
									  filled ? sf::Color(200, 170, 30) : sf::Color::Transparent,
									  sf::Color(60, 50, 45, 150), 0.15f, filled,
									  (unsigned int)(i * 100 + s));
			}
		}

		window.Draw(m_levelScores[i]);
	}

	window.Draw(m_backHint);

	// Konami cheat feedback text
	if (m_cheatTextTimer > 0.0f)
	{
		float alpha = std::min(1.0f, m_cheatTextTimer / 0.5f);
		m_cheatText.setFillColor(sf::Color(200, 170, 30, (sf::Uint8)(255 * alpha)));
		window.Draw(m_cheatText);
	}
}
