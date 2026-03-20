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

// Ink trail mark left behind by the snake's tail
struct InkMark
{
	sf::Vector2f pos;
	float age;
	float maxAge;
};

class Snake
{
public:
	Snake();
	~Snake();

	void Extend();
	void Tick(sf::Vector2u l_windowSize);
	void Render(Window& l_window);
	void RenderInk(sf::RenderTarget& l_target); // Ink-style rendering to arbitrary target
	void Reset();

	// Update ink trail and interpolation state
	void UpdateVisuals(float l_dt);

	inline void SetDirection(Direction l_dir)
	{
		m_dir = l_dir;
	}
	inline Direction GetDirection() const
	{
		return m_dir;
	}
	inline float GetSpeed() const
	{
		return m_speed;
	}
	inline Position GetPosition() const
	{
		return m_headPos;
	}
	inline float GetBlockSize() const
	{
		return m_blockSize;
	}
	inline bool HasLost() const
	{
		return m_lose;
	}
	inline void LoseStatus(bool l_state)
	{
		m_lose = l_state;
	}
	inline bool DidSelfCollide() const
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
	inline int GetBodySize() const
	{
		return (int)m_snakeBody.size();
	}
	inline const std::vector<Position>& GetBody() const
	{
		return m_snakeBody;
	}
	void SetColors(sf::Color l_head, sf::Color l_body)
	{
		m_headColor = l_head;
		m_bodyColor = l_body;
	}

	// Set ink-style visual parameters
	void SetCorruption(float l_corruption) { m_corruption = l_corruption; }
	void SetInkTint(const sf::Color& l_tint) { m_inkTint = l_tint; }
	void SetUseInkStyle(bool l_use) { m_useInkStyle = l_use; }

private:
	void Move(sf::Vector2u l_windowSize);
	void CheckCollision();
	void RenderClassic(Window& l_window); // Original flat rendering
	void RenderInkStyle(sf::RenderTarget& l_target); // New ink aesthetic

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

	sf::Color m_headColor;
	sf::Color m_bodyColor;

	// Ink style visuals
	bool m_useInkStyle;
	float m_corruption;
	sf::Color m_inkTint;
	std::vector<InkMark> m_inkTrail;
	std::vector<Position> m_prevPositions; // For smooth interpolation
	float m_interpTimer; // Time since last tick for interpolation
};
