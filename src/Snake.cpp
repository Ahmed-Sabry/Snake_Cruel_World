#include "Snake.h"
#include "InkRenderer.h"
#include <cmath>
#include <algorithm>

Snake::Snake()
{
	m_blockSize = 16;
	m_speed = 12;
	m_headColor = sf::Color::Red;
	m_bodyColor = sf::Color::Magenta;
	m_useInkStyle = false;
	m_corruption = 0.05f;
	m_inkTint = sf::Color(60, 50, 45);
	m_interpTimer = 0.0f;

	m_bodyRect.setSize({ m_blockSize - 1, m_blockSize - 1 });

	Reset();
}

Snake::~Snake()
{
}

void Snake::Reset()
{
	m_snakeBody.clear();

	m_lose = false;
	m_selfCollided = false;
	m_lastCutSegments.clear();
	m_dir = Direction::None;
	m_inkTrail.clear();
	m_prevPositions.clear();
	m_interpTimer = 0.0f;

	/***************************************************/
	/// Note that Range is:
	/// x = 0 : (windowSize.x / BlockSize)
	/// y = 0 : (windowSize.y / BlockSize)
	/// At least you must Initialize head.
	m_snakeBody.push_back({ 7, 5 }); // Head		/* Must be Initialized */
	/***************************************************/

	m_headPos = m_snakeBody[0]; // Update Initial value for head position
}

void Snake::Move(sf::Vector2u l_windowSize)
{
	// Save previous positions for interpolation
	m_prevPositions = m_snakeBody;

	for (int i = m_snakeBody.size() - 1; i > 0; i--)
	{
		m_snakeBody[i] = m_snakeBody[i - 1];
	}

	switch (m_dir)
	{
		case Direction::Up:
			m_snakeBody[0].y--;
			if (m_snakeBody[0].y < 0)
			{
				m_snakeBody[0].y = l_windowSize.y / m_blockSize;
			}
			break;

		case Direction::Down:
			m_snakeBody[0].y++;
			if (m_snakeBody[0].y > (int)(l_windowSize.y / m_blockSize))
			{
				m_snakeBody[0].y = 0;
			}
			break;

		case Direction::Right:
			m_snakeBody[0].x++;
			if (m_snakeBody[0].x > (int)(l_windowSize.x / m_blockSize))
			{
				m_snakeBody[0].x = 0;
			}
			break;

		case Direction::Left:
			m_snakeBody[0].x--;
			if (m_snakeBody[0].x < 0)
			{
				m_snakeBody[0].x = l_windowSize.x / m_blockSize;
			}
			break;
		default:
			break;
	}

	m_headPos = m_snakeBody[0]; // Update head position
	m_interpTimer = 0.0f; // Reset interpolation on tick

	// Add ink mark at tail position
	if (m_useInkStyle && m_prevPositions.size() > 0)
	{
		auto& tail = m_prevPositions.back();
		InkMark mark;
		mark.pos = sf::Vector2f(tail.x * m_blockSize + m_blockSize * 0.25f,
								tail.y * m_blockSize + m_blockSize * 0.25f);
		mark.age = 0.0f;
		mark.maxAge = 2.0f;
		m_inkTrail.push_back(mark);

		// Limit trail size
		if (m_inkTrail.size() > 60)
			m_inkTrail.erase(m_inkTrail.begin());
	}
}

void Snake::CheckCollision()
{
	if ((int)m_snakeBody.size() < 4)
		return;

	for (int i = 3; i < (int)m_snakeBody.size(); i++)
	{
		if ((m_headPos.x == m_snakeBody[i].x) && (m_headPos.y == m_snakeBody[i].y)) // If snake eat herself
		{
			// Capture cut segments for particle effects
			m_lastCutSegments.assign(m_snakeBody.begin() + i, m_snakeBody.end());
			// Cut
			m_snakeBody.erase(m_snakeBody.begin() + i, m_snakeBody.end());
			m_selfCollided = true;
			break;
		}
	}
}

void Snake::Tick(sf::Vector2u l_windowSize)
{
	Move(l_windowSize);
	CheckCollision();
}

void Snake::Extend()
{
	if ((int)m_snakeBody.size() > 1)
	{
		Position tail = m_snakeBody.back();
		Position pretail = m_snakeBody[m_snakeBody.size() - 2];

		if (tail.x == pretail.x)
		{
			if (tail.y > pretail.y)
				m_snakeBody.push_back({ tail.x, tail.y - 1 });
			else
				m_snakeBody.push_back({ tail.x, tail.y + 1 });
		}
		else if (tail.y == pretail.y)
		{
			if (tail.x > pretail.x)
				m_snakeBody.push_back({ tail.x + 1, tail.y });
			else
				m_snakeBody.push_back({ tail.x - 1, tail.y });
		}
	}
	else
	{
		switch (m_dir)
		{
			case Direction::Up:
				m_snakeBody.push_back({ m_headPos.x, m_headPos.y + 1 });
				break;

			case Direction::Down:
				m_snakeBody.push_back({ m_headPos.x, m_headPos.y - 1 });
				break;

			case Direction::Right:
				m_snakeBody.push_back({ m_headPos.x - 1, m_headPos.y });
				break;

			case Direction::Left:
				m_snakeBody.push_back({ m_headPos.x + 1, m_headPos.y });
				break;
			default:
				break;
		}
	}
}

void Snake::UpdateVisuals(float l_dt)
{
	m_interpTimer += l_dt;

	// Update ink trail
	for (auto& mark : m_inkTrail)
		mark.age += l_dt;

	m_inkTrail.erase(
		std::remove_if(m_inkTrail.begin(), m_inkTrail.end(),
			[](const InkMark& m) { return m.age >= m.maxAge; }),
		m_inkTrail.end());
}

void Snake::Render(Window& l_window)
{
	if (m_useInkStyle)
		RenderInkStyle(l_window.GetRenderWindow());
	else
		RenderClassic(l_window);
}

void Snake::RenderInk(sf::RenderTarget& l_target)
{
	if (m_useInkStyle)
		RenderInkStyle(l_target);
}

void Snake::RenderClassic(Window& l_window)
{
	// Draw Head
	m_bodyRect.setFillColor(m_headColor);
	m_bodyRect.setPosition(m_snakeBody[0].x * m_blockSize, m_snakeBody[0].y * m_blockSize);
	l_window.Draw(m_bodyRect);

	// Draw Body
	m_bodyRect.setFillColor(m_bodyColor);
	for (int i = 1; i < (int)m_snakeBody.size(); i++)
	{
		m_bodyRect.setPosition(m_snakeBody[i].x * m_blockSize, m_snakeBody[i].y * m_blockSize);
		l_window.Draw(m_bodyRect);
	}
}

void Snake::RenderInkStyle(sf::RenderTarget& l_target)
{
	float bs = m_blockSize;
	int bodySize = (int)m_snakeBody.size();

	// Interpolation factor (reserved for smooth movement)
	// float interpT = std::min(1.0f, m_interpTimer * m_speed);

	// --- Draw ink trail ---
	for (const auto& mark : m_inkTrail)
	{
		float alpha = 1.0f - (mark.age / mark.maxAge);
		sf::Uint8 a = (sf::Uint8)(alpha * 60.0f);
		sf::Color trailColor(m_inkTint.r, m_inkTint.g, m_inkTint.b, a);
		sf::RectangleShape trailRect(sf::Vector2f(bs * 0.5f, bs * 0.5f));
		trailRect.setPosition(mark.pos);
		trailRect.setFillColor(trailColor);
		l_target.draw(trailRect);
	}

	// --- Draw connecting lines between segments ---
	for (int i = 0; i < bodySize - 1; i++)
	{
		float x1 = m_snakeBody[i].x * bs + bs * 0.5f;
		float y1 = m_snakeBody[i].y * bs + bs * 0.5f;
		float x2 = m_snakeBody[i + 1].x * bs + bs * 0.5f;
		float y2 = m_snakeBody[i + 1].y * bs + bs * 0.5f;

		// Skip if segments are too far apart (wrapping)
		float dx = std::abs(x2 - x1);
		float dy = std::abs(y2 - y1);
		if (dx > bs * 2 || dy > bs * 2) continue;

		sf::Uint8 alpha = (sf::Uint8)(200 - (i * 50 / std::max(1, bodySize)));
		sf::Color lineColor(m_inkTint.r, m_inkTint.g, m_inkTint.b, alpha);

		sf::Vertex line[] = {
			sf::Vertex(sf::Vector2f(x1, y1), lineColor),
			sf::Vertex(sf::Vector2f(x2, y2), lineColor)
		};
		l_target.draw(line, 2, sf::Lines);
	}

	// --- Draw body segments (back to front for proper overlap) ---
	for (int i = bodySize - 1; i >= 1; i--)
	{
		float x = m_snakeBody[i].x * bs;
		float y = m_snakeBody[i].y * bs;

		// Alpha gradient: fade toward tail
		float alphaFrac = 1.0f - ((float)(i - 1) / std::max(1.0f, (float)(bodySize - 1))) * 0.25f;
		sf::Uint8 bodyAlpha = (sf::Uint8)(255 * alphaFrac);

		// Alternating fill color with slight variation
		sf::Color fillColor(m_bodyColor.r, m_bodyColor.g, m_bodyColor.b, bodyAlpha);
		sf::Color outlineColor(m_inkTint.r, m_inkTint.g, m_inkTint.b, bodyAlpha);

		float segSize = bs - 2.0f;
		float segOffset = 1.0f;

		InkRenderer::DrawWobblyRect(l_target,
									x + segOffset, y + segOffset, segSize, segSize,
									fillColor, outlineColor, 1.0f,
									m_corruption,
									(unsigned int)(i * 73 + 17)); // Stable seed per segment
	}

	// --- Draw head ---
	{
		float hx = m_snakeBody[0].x * bs;
		float hy = m_snakeBody[0].y * bs;

		// Head is slightly larger and more prominent
		sf::Color headFill = m_headColor;
		sf::Color headOutline(m_inkTint.r, m_inkTint.g, m_inkTint.b, 255);

		InkRenderer::DrawWobblyRect(l_target,
									hx, hy, bs - 1, bs - 1,
									headFill, headOutline, 1.5f,
									m_corruption, 7);

		// --- Draw eyes ---
		float eyeRadius = 2.5f;
		float eyeOffsetForward = bs * 0.2f;
		float eyeOffsetSide = bs * 0.2f;

		float cx = hx + bs * 0.5f;
		float cy = hy + bs * 0.5f;
		float ex1 = cx, ey1 = cy, ex2 = cx, ey2 = cy;

		switch (m_dir)
		{
			case Direction::Up:
				ex1 = cx - eyeOffsetSide; ey1 = cy - eyeOffsetForward;
				ex2 = cx + eyeOffsetSide; ey2 = cy - eyeOffsetForward;
				break;
			case Direction::Down:
				ex1 = cx - eyeOffsetSide; ey1 = cy + eyeOffsetForward;
				ex2 = cx + eyeOffsetSide; ey2 = cy + eyeOffsetForward;
				break;
			case Direction::Left:
				ex1 = cx - eyeOffsetForward; ey1 = cy - eyeOffsetSide;
				ex2 = cx - eyeOffsetForward; ey2 = cy + eyeOffsetSide;
				break;
			case Direction::Right:
				ex1 = cx + eyeOffsetForward; ey1 = cy - eyeOffsetSide;
				ex2 = cx + eyeOffsetForward; ey2 = cy + eyeOffsetSide;
				break;
			default:
				ex1 = cx - eyeOffsetSide; ey1 = cy - eyeOffsetForward;
				ex2 = cx + eyeOffsetSide; ey2 = cy - eyeOffsetForward;
				break;
		}

		sf::CircleShape eye(eyeRadius);
		eye.setOrigin(eyeRadius, eyeRadius);
		eye.setFillColor(headOutline);

		eye.setPosition(ex1, ey1);
		l_target.draw(eye);
		eye.setPosition(ex2, ey2);
		l_target.draw(eye);

		// Pupil (tiny white dot)
		sf::CircleShape pupil(1.0f);
		pupil.setOrigin(1.0f, 1.0f);
		pupil.setFillColor(sf::Color::White);

		pupil.setPosition(ex1, ey1);
		l_target.draw(pupil);
		pupil.setPosition(ex2, ey2);
		l_target.draw(pupil);
	}
}
