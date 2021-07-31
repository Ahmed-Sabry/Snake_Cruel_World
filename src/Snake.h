#pragma once

#include "List.h"
#include "Window.h"

enum class Direction
{
	None,
	Up,
	Down,
	Right,
	Left
};

class Snake
{
public:
	Snake();
	~Snake();

	void Extend();
	void Tick(sf::Vector2u l_windowSize);
	void Render(Window& l_window);
	void Reset();

	inline void SetDirection(Direction l_dir)
	{
		m_dir = l_dir;
	}
	inline Direction GetDirection()
	{
		return m_dir;
	}
	inline float GetSpeed()
	{
		return m_speed;
	}
	inline Position GetPosition()
	{
		return m_headPos;
	}
	inline float GetBlockSize()
	{
		return m_blockSize;
	}
	inline bool HasLost()
	{
		return m_lose;
	}
	inline void LoseStatus(bool l_state)
	{
		m_lose = l_state;
	}

private:
	void Move(sf::Vector2u l_windowSize);
	void CheckCollision();

private:
	List m_snakeBody;
	Position m_pos;
	Position m_headPos; // Snake head position
	Direction m_dir;
	float m_speed;
	sf::RectangleShape m_bodyRect;

	bool m_lose;

	float m_blockSize;
};