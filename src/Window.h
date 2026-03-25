#pragma once

#include "Platform/Platform.hpp"
#include <array>
#include <iostream>
#include <functional>
#include <vector>

class Window
{
public:
	Window(const sf::Vector2u l_windowSize = { 640, 480 }, std::string l_windowName = "Window");
	~Window();

	void Update();
	bool IsKeyPressed(sf::Keyboard::Key l_key) const;

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
	inline void Close()
	{
		m_isDone = true;
	}
	inline sf::Vector2u GetWindowSize()
	{
		return m_windowSize;
	}
	inline sf::RenderWindow& GetRenderWindow()
	{
		return m_window;
	}
	inline void SetView(const sf::View& l_view)
	{
		m_window.setView(l_view);
	}
	inline sf::View GetDefaultView()
	{
		return m_window.getDefaultView();
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
	std::array<bool, sf::Keyboard::KeyCount> m_keyStates{};
};