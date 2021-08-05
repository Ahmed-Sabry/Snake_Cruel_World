#pragma once

#include "Platform/Platform.hpp"
#include "Snake.h"
#include "Textbox.h"
#include "Window.h"

class World
{
public:
	World(Window& l_window, Snake& l_snake, Textbox* l_log);
	~World();

	void Reset(Window& l_window, Snake& l_snake);
	void Update(Window& l_window, Snake& l_snake);
	void Render(Window& l_window);
	void Borders(Window& l_window);
	void RespawnApple(Snake& l_snake);
	void NarrowWorld(Window& l_window, Snake& l_snake);
	void CheckCollision(Window& l_window, Snake& l_snake);

	inline sf::Vector2f GetApplePos()
	{
		return m_applePos;
	}

private:
	sf::RectangleShape m_borders[4];
	sf::CircleShape m_apple;

	sf::Vector2f m_applePos;

	int m_count;

	float m_maxX;
	float m_maxY;
	float m_appleRaduis;
	float m_borderThinkness;

	Textbox* m_log;
};

inline float Random(int a, int b)
{
	return (a + rand() % (b - a + 1));
}
