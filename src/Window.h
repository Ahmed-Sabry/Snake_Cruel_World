#pragma once

#include <SFML\Graphics.hpp>
#include <iostream>

class Window
{
public:
	Window(const sf::Vector2u l_windowSize = { 640, 480 }, std::string l_windowName = "Window");
	~Window();

	void Update();

	inline void SetBackground(sf::Color l_color)
	{
		m_backgroundColor = l_color;
	}
	inline void Draw(sf::Drawable& l_drawable)
	{
		m_window.draw(l_drawable);
	}
	inline void Clear()
	{
		m_window.clear(m_backgroundColor);
	}
	inline void Display()
	{
		m_window.display();
	}
	inline bool IsDone()
	{
		return m_isDone;
	}
	inline sf::Vector2u GetWindowSize()
	{
		return m_windowSize;
	}

private:
	void ToggleFullscreen();
	void Create();

private:
	sf::Vector2u m_windowSize;
	std::string m_windowName;
	sf::Color m_backgroundColor;
	bool m_isDone;
	bool m_isFullscreen;
	sf::RenderWindow m_window;
	sf::Event m_event;
};