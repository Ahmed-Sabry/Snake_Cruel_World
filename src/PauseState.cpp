#include "PauseState.h"
#include "LevelConfig.h"
#include <cstdlib>
#include <iostream>

const char* PauseState::s_cruelTips[] = {
	"The walls are closer than they appear.",
	"Fun fact: the apple is not your friend.",
	"This level was designed by someone who hates you.",
	"Taking a break won't make it easier.",
	"The world shrinks whether you're ready or not.",
	"Your high score is watching. Judgmentally.",
	"Every second paused is a second not dying.",
	"The borders are patient. They can wait.",
	"You look nervous. Good.",
	"Remember: the game always wins eventually.",
	"Pausing only delays the inevitable.",
	"The snake doesn't get tired. But you do.",
	"Think of this as a timeout. You'll need it.",
	"Behind you! Just kidding. But soon.",
	"The difficulty curve has no mercy.",
	"Somewhere, a wall just got closer.",
	"Rest now. The cruelty resumes shortly.",
	"You can pause, but you can't hide.",
	"The game remembers every mistake you've made.",
	"Confidence is the last thing you lose. Before everything.",
	"The apple doesn't care about your feelings.",
	"Some call it a game. We call it natural selection.",
	"Quitting is always an option. A cowardly one.",
	"The snake was happier before you started controlling it.",
	"Pro tip: don't hit the walls."
};

const int PauseState::s_tipCount = 25;

PauseState::PauseState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedItem(0),
	  m_itemCount(3),
	  m_keyReleased(false) // start false to prevent immediate unpause
{
}

void PauseState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "PauseState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();

	// Dark overlay
	m_overlay.setSize(sf::Vector2f((float)winSize.x, (float)winSize.y));
	m_overlay.setFillColor(sf::Color(0, 0, 0, 160));
	m_overlay.setPosition(0, 0);

	// Title
	m_title.setFont(m_font);
	m_title.setString("PAUSED");
	m_title.setCharacterSize(48);
	m_title.setFillColor(sf::Color(200, 50, 50));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 200.f);

	// Cruel tip
	m_cruelTip.setFont(m_font);
	m_cruelTip.setString(s_cruelTips[rand() % s_tipCount]);
	m_cruelTip.setCharacterSize(18);
	m_cruelTip.setFillColor(sf::Color(160, 120, 120));
	sf::FloatRect tipBounds = m_cruelTip.getLocalBounds();
	m_cruelTip.setPosition((winSize.x - tipBounds.width) / 2.0f, 280.f);

	// Menu items
	std::string items[] = { "Resume", "Restart", "Quit to Menu" };
	float startY = 380.f;
	float spacing = 50.f;

	for (int i = 0; i < m_itemCount; i++)
	{
		m_menuItems[i].setFont(m_font);
		m_menuItems[i].setString(items[i]);
		m_menuItems[i].setCharacterSize(28);
		sf::FloatRect bounds = m_menuItems[i].getLocalBounds();
		m_menuItems[i].setPosition((winSize.x - bounds.width) / 2.0f, startY + i * spacing);
	}

	m_selectedItem = 0;
	m_keyReleased = false; // must release key first
}

void PauseState::OnExit()
{
}

void PauseState::HandleInput()
{
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	bool escPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);

	if (!upPressed && !downPressed && !enterPressed && !escPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;

	if (escPressed)
	{
		m_stateManager.PopState(); // back to gameplay
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
			case 0: // Resume
				m_stateManager.PopState();
				break;
			case 1: // Restart
				m_stateManager.PopState();
				m_stateManager.SwitchTo(StateType::Gameplay);
				break;
			case 2: // Quit to Menu
				m_stateManager.PopState();
				m_stateManager.SwitchTo(StateType::MainMenu);
				break;
			default:
				break;
		}
	}
}

void PauseState::Update(float l_dt)
{
	UNUSED(l_dt);

	for (int i = 0; i < m_itemCount; i++)
	{
		if (i == m_selectedItem)
			m_menuItems[i].setFillColor(sf::Color(255, 100, 80));
		else
			m_menuItems[i].setFillColor(sf::Color(180, 170, 170));
	}
}

void PauseState::Render()
{
	// Don't clear - render on top of gameplay
	Window& window = m_stateManager.GetWindow();
	window.Draw(m_overlay);
	window.Draw(m_title);
	window.Draw(m_cruelTip);

	for (int i = 0; i < m_itemCount; i++)
		window.Draw(m_menuItems[i]);
}
