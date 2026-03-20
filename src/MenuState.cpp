#include "MenuState.h"
#include "AudioManager.h"
#include "LevelConfig.h"
#include "InkRenderer.h"
#include <algorithm>
#include <iostream>

MenuState::MenuState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedItem(0),
	  m_itemCount(3),
	  m_keyReleased(true),
	  m_animTimer(0.0f)
{
}

void MenuState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "MenuState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();

	// Generate a "menu page" paper background using Level 1's palette (warm, inviting)
	LevelConfig menuConfig{};
	menuConfig.id = 0; // Special: menu
	menuConfig.paperTone = sf::Color(245, 235, 220);
	menuConfig.inkTint = sf::Color(60, 50, 45);
	menuConfig.corruption = 0.02f;
	m_paperBg.Generate(menuConfig, winSize.x, winSize.y);

	// Use paper tone as window bg fallback
	window.SetBackground(menuConfig.paperTone);

	// Title - hand-drawn feel with the handwriting font
	m_title.setFont(m_font);
	m_title.setString("Hello Cruel World");
	m_title.setCharacterSize(56);
	m_title.setFillColor(sf::Color(180, 50, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 100.f);

	// Tagline - slight rotation like an afterthought
	m_tagline.setFont(m_font);
	m_tagline.setString("The world doesn't just get smaller. It gets meaner.");
	m_tagline.setCharacterSize(18);
	m_tagline.setFillColor(sf::Color(120, 90, 80));
	sf::FloatRect tagBounds = m_tagline.getLocalBounds();
	m_tagline.setPosition((winSize.x - tagBounds.width) / 2.0f, 175.f);
	m_tagline.setRotation(1.0f); // Slight tilt

	// Menu items with bracket markers
	std::string items[] = { "Play", "Level Select", "Quit" };
	float startY = 320.f;
	float spacing = 60.f;

	for (int i = 0; i < m_itemCount; i++)
	{
		m_menuItems[i].setFont(m_font);
		m_menuItems[i].setString(items[i]);
		m_menuItems[i].setCharacterSize(32);

		sf::FloatRect bounds = m_menuItems[i].getLocalBounds();
		m_menuItems[i].setPosition((winSize.x - bounds.width) / 2.0f, startY + i * spacing);
	}

	m_selectedItem = 0;
	m_keyReleased = true;

	m_stateManager.GetAudio().PlayMusic("content/audio/music/menu_theme.ogg");
}

void MenuState::OnExit()
{
	m_stateManager.GetAudio().StopMusic();
}

void MenuState::HandleInput()
{
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

	if (!upPressed && !downPressed && !enterPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;

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
		switch (m_selectedItem)
		{
			case 0: // Play
				m_stateManager.currentLevel = 1;
				m_stateManager.SwitchTo(StateType::Gameplay);
				break;
			case 1: // Level Select
				m_stateManager.SwitchTo(StateType::LevelSelect);
				break;
			case 2: // Quit
				m_stateManager.GetWindow().Close();
				break;
			default:
				break;
		}
	}
}

void MenuState::Update(float l_dt)
{
	m_animTimer += l_dt;

	sf::Color inkColor(60, 50, 45);
	sf::Color selectedInk(180, 50, 40);

	for (int i = 0; i < m_itemCount; i++)
	{
		if (i == m_selectedItem)
		{
			// Pulsing ink intensity
			float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
			int r = 160 + (int)(40 * pulse);
			int g = 40 + (int)(20 * pulse);
			int b = 30;
			m_menuItems[i].setFillColor(sf::Color(r, g, b));
			m_menuItems[i].setString("> " + std::string(
				i == 0 ? "Play" : (i == 1 ? "Level Select" : "Quit")));
			sf::FloatRect bounds = m_menuItems[i].getLocalBounds();
			float winWidth = (float)m_stateManager.GetWindow().GetWindowSize().x;
			m_menuItems[i].setPosition((winWidth - bounds.width) / 2.0f, m_menuItems[i].getPosition().y);
		}
		else
		{
			m_menuItems[i].setFillColor(sf::Color(100, 90, 85));
			std::string items[] = { "Play", "Level Select", "Quit" };
			m_menuItems[i].setString("  " + items[i]);
			sf::FloatRect bounds = m_menuItems[i].getLocalBounds();
			float winWidth = (float)m_stateManager.GetWindow().GetWindowSize().x;
			m_menuItems[i].setPosition((winWidth - bounds.width) / 2.0f, m_menuItems[i].getPosition().y);
		}
	}
}

void MenuState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	// Paper background
	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	// Wobbly underline below title
	sf::FloatRect titleBounds = m_title.getGlobalBounds();
	InkRenderer::DrawWobblyLine(target,
								titleBounds.left, titleBounds.top + titleBounds.height + 8,
								titleBounds.left + titleBounds.width, titleBounds.top + titleBounds.height + 8,
								sf::Color(180, 50, 40, 150), 2.0f, 0.15f,
								(unsigned int)(m_animTimer * 0.5f)); // Slowly changing wobble

	window.Draw(m_title);
	window.Draw(m_tagline);

	for (int i = 0; i < m_itemCount; i++)
	{
		window.Draw(m_menuItems[i]);

		// Draw selection circle around selected item
		if (i == m_selectedItem)
		{
			sf::FloatRect bounds = m_menuItems[i].getGlobalBounds();
			float cx = bounds.left + bounds.width * 0.5f;
			float cy = bounds.top + bounds.height * 0.5f;
			float rx = bounds.width * 0.6f + 10.0f;
			float ry = bounds.height * 0.5f + 8.0f;

			// Animated wobble circle (different seed each frame for nervous feel)
			InkRenderer::DrawWobblyCircle(target, cx, cy,
										  std::max(rx, ry),
										  sf::Color::Transparent,
										  sf::Color(180, 50, 40, 80),
										  1.0f, 0.3f,
										  (unsigned int)(m_animTimer * 10.0f), 16);
		}
	}

	// Small snake doodle in bottom-right corner
	float dx = (float)window.GetWindowSize().x - 60.0f;
	float dy = (float)window.GetWindowSize().y - 50.0f;
	sf::Color doodleColor(60, 50, 45, 80);

	// Snake body (3 small connected rectangles)
	for (int s = 0; s < 3; s++)
	{
		InkRenderer::DrawWobblyRect(target,
									dx + s * 12.0f, dy, 10, 10,
									doodleColor, doodleColor, 0.5f, 0.1f, 500 + s);
	}
	// Arrow pointing to it
	InkRenderer::DrawWobblyLine(target, dx + 40, dy + 5, dx + 55, dy + 5,
								doodleColor, 1.0f, 0.1f, 510);
}
