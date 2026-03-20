#include "MirrorGhost.h"
#include "InkRenderer.h"
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

	sf::RenderTarget& target = l_window.GetRenderWindow();

	// Draw dashed axis line showing the mirror plane
	float centerX = ((float)l_boundsMinX + (float)l_boundsMaxX) * 0.5f * l_blockSize;
	float centerY = ((float)l_boundsMinY + (float)l_boundsMaxY) * 0.5f * l_blockSize;

	sf::Color axisColor(15, 15, 15, 30);
	if (m_axis == MirrorAxis::Vertical)
	{
		// Vertical dashed line
		for (float y = l_boundsMinY * l_blockSize; y < l_boundsMaxY * l_blockSize; y += 12.0f)
		{
			sf::Vertex dash[] = {
				sf::Vertex(sf::Vector2f(centerX, y), axisColor),
				sf::Vertex(sf::Vector2f(centerX, y + 6.0f), axisColor)
			};
			target.draw(dash, 2, sf::Lines);
		}
	}
	else
	{
		// Horizontal dashed line
		for (float x = l_boundsMinX * l_blockSize; x < l_boundsMaxX * l_blockSize; x += 12.0f)
		{
			sf::Vertex dash[] = {
				sf::Vertex(sf::Vector2f(x, centerY), axisColor),
				sf::Vertex(sf::Vector2f(x + 6.0f, centerY), axisColor)
			};
			target.draw(dash, 2, sf::Lines);
		}
	}

	// Draw ghost with inverted style (dotted outline, photographic-negative feel)
	for (size_t i = 0; i < m_ghostBody.size(); i++)
	{
		const Position& seg = m_ghostBody[i];
		if (seg.x < l_boundsMinX || seg.x > l_boundsMaxX ||
			seg.y < l_boundsMinY || seg.y > l_boundsMaxY)
			continue;

		float px = seg.x * l_blockSize;
		float py = seg.y * l_blockSize;
		float segSize = l_blockSize - 2.0f;

		sf::Uint8 alpha = (i == 0) ? (sf::Uint8)130 : (sf::Uint8)80;

		// Inverted style: white fill on dark background impression
		sf::Color ghostFill(240, 240, 240, (sf::Uint8)(alpha * 0.4f));
		sf::Color ghostOutline(15, 15, 15, alpha);

		InkRenderer::DrawWobblyRect(target,
									px + 1.0f, py + 1.0f, segSize, segSize,
									ghostFill, ghostOutline,
									1.0f, 0.25f,
									(unsigned int)(i * 53 + 200));
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
