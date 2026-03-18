#pragma once

#include "Window.h"
#include <vector>

struct Position
{
	int x;
	int y;
};

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
	inline bool DidSelfCollide()
	{
		return m_selfCollided;
	}
	inline void ClearSelfCollideFlag()
	{
		m_selfCollided = false;
		m_lastCutSegments.clear();
	}
	inline const std::vector<Position>& GetLastCutSegments() const
	{
		return m_lastCutSegments;
	}
	inline void SetSpeed(float l_speed)
	{
		m_speed = l_speed;
	}
	inline int GetBodySize()
	{
		return (int)m_snakeBody.size();
	}

private:
	void Move(sf::Vector2u l_windowSize);
	void CheckCollision();

private:
	std::vector<Position> m_snakeBody;
	Position m_headPos; // Snake head position
	Direction m_dir;
	float m_speed;
	sf::RectangleShape m_bodyRect;

	bool m_lose;
	bool m_selfCollided;
	std::vector<Position> m_lastCutSegments;

	float m_blockSize;
};
