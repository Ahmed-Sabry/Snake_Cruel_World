#include "MirrorGhost.h"
#include <cmath>

MirrorGhost::MirrorGhost()
	: m_axis(MirrorAxis::Vertical)
{
}

void MirrorGhost::Reset()
{
	m_axis = MirrorAxis::Vertical;
	m_history.clear();
	m_ghostBody.clear();
}

void MirrorGhost::Update(const Snake& l_snake, float l_centerX, float l_centerY)
{
	// Record current snake head position (raw, un-mirrored)
	m_history.push_back(l_snake.GetPosition());

	// Need enough history before the ghost appears
	// Total snake length = head + body segments
	int bodySize = l_snake.GetBodySize();
	int totalSize = bodySize + 1;
	int needed = GHOST_DELAY + totalSize;
	if ((int)m_history.size() < needed)
	{
		m_ghostBody.clear();
		return;
	}

	// Trim old history we'll never need
	while ((int)m_history.size() > needed + 10)
		m_history.pop_front();

	// Build ghost body from delayed history, mirrored with current axis
	m_ghostBody.resize(totalSize);
	int histEnd = (int)m_history.size() - 1;
	for (int i = 0; i < totalSize; i++)
	{
		int idx = histEnd - GHOST_DELAY - i;
		if (idx >= 0)
			m_ghostBody[i] = MirrorPosition(m_history[idx], l_centerX, l_centerY);
	}
}

Position MirrorGhost::MirrorPosition(const Position& l_pos, float l_centerX, float l_centerY) const
{
	Position mirrored;
	if (m_axis == MirrorAxis::Vertical)
	{
		mirrored.x = (int)std::round(2.0f * l_centerX - l_pos.x);
		mirrored.y = l_pos.y;
	}
	else
	{
		mirrored.x = l_pos.x;
		mirrored.y = (int)std::round(2.0f * l_centerY - l_pos.y);
	}
	return mirrored;
}

void MirrorGhost::Render(Window& l_window, float l_blockSize,
						 int l_boundsMinX, int l_boundsMaxX, int l_boundsMinY, int l_boundsMaxY)
{
	if (m_ghostBody.empty()) return;

	m_ghostRect.setSize(sf::Vector2f(l_blockSize - 1, l_blockSize - 1));

	for (size_t i = 0; i < m_ghostBody.size(); i++)
	{
		const Position& seg = m_ghostBody[i];
		// Skip segments outside playable bounds (can happen after world shrinks)
		if (seg.x < l_boundsMinX || seg.x > l_boundsMaxX ||
			seg.y < l_boundsMinY || seg.y > l_boundsMaxY)
			continue;

		m_ghostRect.setFillColor(i == 0
			? sf::Color(0, 255, 255, 120)   // head
			: sf::Color(0, 255, 255, 80));   // body
		m_ghostRect.setPosition(seg.x * l_blockSize, seg.y * l_blockSize);
		l_window.Draw(m_ghostRect);
	}
}

bool MirrorGhost::CheckCollision(const Position& l_snakeHead) const
{
	for (const auto& seg : m_ghostBody)
	{
		if (l_snakeHead.x == seg.x && l_snakeHead.y == seg.y)
			return true;
	}
	return false;
}

void MirrorGhost::FlipAxis()
{
	m_axis = (m_axis == MirrorAxis::Vertical) ? MirrorAxis::Horizontal : MirrorAxis::Vertical;
}

MirrorAxis MirrorGhost::GetAxis() const
{
	return m_axis;
}
