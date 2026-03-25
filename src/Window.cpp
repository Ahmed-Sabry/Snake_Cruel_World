#include "Window.h"

Window::Window(const sf::Vector2u l_windowSize, std::string l_windowName) :
	m_windowSize(l_windowSize),
	m_windowName(l_windowName),
	m_backgroundColor(sf::Color::White),
	m_isDone(false),
	m_isFullscreen(false)

{
	Create();
}

Window::~Window()
{
}

void Window::Create()
{
	auto mode = (m_isFullscreen ? sf::Style::Fullscreen : sf::Style::Default);
	m_window.create({ m_windowSize.x, m_windowSize.y, 32 }, m_windowName, mode);
	m_window.setFramerateLimit(60);
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
				if (m_event.key.code >= 0 && m_event.key.code < sf::Keyboard::KeyCount)
					m_keyStates[(std::size_t)m_event.key.code] = true;
				if (m_event.key.code == sf::Keyboard::F5) /* Press F5 to toggle FullScreen */
					ToggleFullscreen();
				// Escape is now handled by game states (pause, menu, etc.)
				break;

			case sf::Event::KeyReleased:
				if (m_event.key.code >= 0 && m_event.key.code < sf::Keyboard::KeyCount)
					m_keyStates[(std::size_t)m_event.key.code] = false;
				break;

			case sf::Event::LostFocus:
				m_keyStates.fill(false);
				break;

			default:
				break;
		}
	}
}

bool Window::IsKeyPressed(sf::Keyboard::Key l_key) const
{
	if (l_key < 0 || l_key >= sf::Keyboard::KeyCount)
		return false;

	return m_keyStates[(std::size_t)l_key];
}

void Window::ToggleFullscreen()
{
	m_isFullscreen = !m_isFullscreen;
	m_window.close();
	Create();
}
