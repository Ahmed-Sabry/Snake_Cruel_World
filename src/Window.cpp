#include "Window.h"

Window::Window(const sf::Vector2u l_windowSize, std::string l_windowName) :
	m_windowSize(l_windowSize),
	m_windowName(l_windowName),
	m_backgroundColor(sf::Color::White),
	m_isDone(false),
	m_isFullscreen(false)

{
	m_window.close();
	Create();
}

Window::~Window()
{
}

void Window::Create()
{
	auto mode = (m_isFullscreen ? sf::Style::Fullscreen : sf::Style::Default);
	m_window.create({ m_windowSize.x, m_windowSize.y, 32 }, m_windowName, mode);
}

void Window::Update()
{
	while (m_window.pollEvent(m_event))
	{
		switch (m_event.type)
		{
			case sf::Event::Closed: /* The window requested to be closed */
				m_isDone = true;
				break;

			case sf::Event::KeyPressed:
				if (m_event.key.code == sf::Keyboard::F5) /* Press F5 to toggle FullScreen */
					ToggleFullscreen();
				else if (m_event.key.code == sf::Keyboard::Escape) /* Press Esc to close the game */
					m_isDone = true;
				break;

			default:
				break;
		}
	}
}

void Window::ToggleFullscreen()
{
	m_isFullscreen = !m_isFullscreen;
	m_window.close();
	Create();
}