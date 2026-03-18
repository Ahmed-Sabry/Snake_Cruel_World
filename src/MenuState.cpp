#include "MenuState.h"
#include "LevelConfig.h"
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
	window.SetBackground(sf::Color(15, 8, 12));

	// Title
	m_title.setFont(m_font);
	m_title.setString("Hello Cruel World");
	m_title.setCharacterSize(56);
	m_title.setFillColor(sf::Color(200, 50, 50));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 100.f);

	// Tagline
	m_tagline.setFont(m_font);
	m_tagline.setString("The world doesn't just get smaller. It gets meaner.");
	m_tagline.setCharacterSize(18);
	m_tagline.setFillColor(sf::Color(150, 100, 100));
	sf::FloatRect tagBounds = m_tagline.getLocalBounds();
	m_tagline.setPosition((winSize.x - tagBounds.width) / 2.0f, 175.f);

	// Menu items
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
}

void MenuState::OnExit()
{
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

	// Update menu item colors based on selection
	for (int i = 0; i < m_itemCount; i++)
	{
		if (i == m_selectedItem)
		{
			// Pulsing red glow
			float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
			int r = 180 + (int)(75 * pulse);
			int g = 40 + (int)(30 * pulse);
			int b = 40;
			m_menuItems[i].setFillColor(sf::Color(r, g, b));
		}
		else
		{
			m_menuItems[i].setFillColor(sf::Color(140, 130, 130));
		}
	}
}

void MenuState::Render()
{
	Window& window = m_stateManager.GetWindow();

	window.Draw(m_title);
	window.Draw(m_tagline);

	for (int i = 0; i < m_itemCount; i++)
		window.Draw(m_menuItems[i]);
}
