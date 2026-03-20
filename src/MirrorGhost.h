#pragma once

#include "Window.h"
#include "Snake.h"
#include <vector>
#include <deque>

enum class MirrorAxis
{
	Vertical,
	Horizontal
};

class MirrorGhost
{
public:
	static constexpr int GHOST_DELAY = 5;

	MirrorGhost();

	void Reset();
	void Update(const Snake& l_snake, float l_centerX, float l_centerY);
	void Render(Window& l_window, float l_blockSize,
			   int l_boundsMinX, int l_boundsMaxX, int l_boundsMinY, int l_boundsMaxY);
	void RenderTo(sf::RenderTarget& l_target, float l_blockSize,
				  int l_boundsMinX, int l_boundsMaxX, int l_boundsMinY, int l_boundsMaxY);
	bool CheckCollision(const Position& l_snakeHead) const;
	void FlipAxis();
	MirrorAxis GetAxis() const;

private:
	Position MirrorPosition(const Position& l_pos, float l_centerX, float l_centerY) const;

	std::deque<Position> m_history;    // raw snake head positions, newest at back
	std::vector<Position> m_ghostBody; // computed mirrored ghost for render/collision
	MirrorAxis m_axis;
	sf::RectangleShape m_ghostRect;
};
