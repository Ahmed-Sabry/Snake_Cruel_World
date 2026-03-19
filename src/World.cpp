#include "World.h"
#include <algorithm>

World::World(Window& l_window, Snake& l_snake)
{
	m_topOffset = 0.f;
	m_flashTimer = 0.0f;
	m_normalBorderColor = sf::Color(200, 100, 50);
	ResetBorderOffsets();
	Reset(l_window, l_snake);

	m_appleRaduis = l_snake.GetBlockSize() / 2;
	m_apple.setRadius(m_appleRaduis);
	m_apple.setFillColor(sf::Color::Green);

	m_maxX = (l_window.GetWindowSize().x / l_snake.GetBlockSize());
	m_maxY = (l_window.GetWindowSize().y / l_snake.GetBlockSize());

	srand(time(nullptr));
}

World::~World()
{
}

void World::Reset(Window& l_window, Snake& l_snake)
{
	m_borderThickness = l_snake.GetBlockSize();
	ResetBorderOffsets();
	Borders(l_window);
	m_count = 0;
	m_totalApplesEaten = 0;
	m_shrinkCount = 0;
	m_shrinkInterval = 4;
	m_shrinkTimerSec = 0.0f;
	m_shrinkTimerAccum = 0.0f;
}

void World::SetBorderColor(sf::Color l_color)
{
	m_normalBorderColor = l_color;
	for (int i = 0; i < 4; i++)
		m_borders[i].setFillColor(l_color);
}

void World::FlashBorders(float l_duration)
{
	if (l_duration <= 0.0f)
		return;

	m_flashTimer = l_duration;
	for (int i = 0; i < 4; i++)
		m_borders[i].setFillColor(sf::Color::Red);
}

void World::UpdateFlash(float l_dt)
{
	if (m_flashTimer <= 0.0f)
		return;

	m_flashTimer -= l_dt;
	if (m_flashTimer <= 0.0f)
	{
		m_flashTimer = 0.0f;
		for (int i = 0; i < 4; i++)
			m_borders[i].setFillColor(m_normalBorderColor);
	}
}

void World::SetTopOffset(float l_offset)
{
	m_topOffset = l_offset;
}

void World::Borders(Window& l_window)
{
	float winWidth = (float)l_window.GetWindowSize().x;
	float winHeight = (float)l_window.GetWindowSize().y;

	float effTop    = m_borderThickness + m_borderOffset[0];
	float effRight  = m_borderThickness + m_borderOffset[1];
	float effBottom = m_borderThickness + m_borderOffset[2];
	float effLeft   = m_borderThickness + m_borderOffset[3];

	for (int i = 0; i < 4; i++)
		m_borders[i].setFillColor(m_normalBorderColor);

	// Top
	m_borders[0].setSize({ winWidth, effTop });
	m_borders[0].setPosition({ 0, m_topOffset });
	// Right
	m_borders[1].setSize({ effRight, winHeight - m_topOffset });
	m_borders[1].setPosition({ winWidth - effRight, m_topOffset });
	// Bottom
	m_borders[2].setSize({ winWidth, effBottom });
	m_borders[2].setPosition({ 0, winHeight - effBottom });
	// Left
	m_borders[3].setSize({ effLeft, winHeight - m_topOffset });
	m_borders[3].setPosition({ 0, m_topOffset });
}

void World::NarrowWorld(Window& l_window, Snake& l_snake)
{
	m_borderThickness += l_snake.GetBlockSize();
	Borders(l_window);
	m_shrinkCount++;

	// If apple is now inside a wall, respawn it within new bounds
	float bs = l_snake.GetBlockSize();
	float xLeft = (m_borderThickness + m_borderOffset[3]) / bs;
	float xRight = m_maxX - (m_borderThickness + m_borderOffset[1]) / bs - 2 * (m_appleRaduis / bs);
	float yTop = (m_borderThickness + m_borderOffset[0] + m_topOffset) / bs;
	float yBottom = m_maxY - (m_borderThickness + m_borderOffset[2]) / bs - 2 * (m_appleRaduis / bs);

	if (m_applePos.x < xLeft || m_applePos.x > xRight ||
		m_applePos.y < yTop || m_applePos.y > yBottom)
	{
		RespawnApple(l_snake);
	}
}

void World::RespawnApple(Snake& l_snake)
{
	float bs = l_snake.GetBlockSize();
	int xMin = (int)((m_borderThickness + m_borderOffset[3]) / bs);
	int xMax = (int)(m_maxX - (m_borderThickness + m_borderOffset[1]) / bs - 2 * (m_appleRaduis / bs));
	int yMin = (int)((m_borderThickness + m_borderOffset[0] + m_topOffset) / bs);
	int yMax = (int)(m_maxY - (m_borderThickness + m_borderOffset[2]) / bs - 2 * (m_appleRaduis / bs));

	// If eating this apple will trigger a shrink, add 1-block inward margin
	// so the snake won't be crushed by the border moving inward
	if (m_shrinkInterval > 0 && m_count == m_shrinkInterval - 1)
	{
		xMin += 1;
		xMax -= 1;
		yMin += 1;
		yMax -= 1;
	}

	// Guard against invalid bounds after excessive shrinking
	if (xMin > xMax) xMin = xMax;
	if (yMin > yMax) yMin = yMax;

	float x = Random(xMin, xMax);
	float y = Random(yMin, yMax);

	m_apple.setPosition(sf::Vector2f(x * bs, y * bs));
	m_applePos = { x, y };
}

void World::CheckCollision(Window& l_window, Snake& l_snake)
{
	float bs = l_snake.GetBlockSize();
	float xLeft = (m_borderThickness + m_borderOffset[3]) / bs;
	float xRight = m_maxX - (m_borderThickness + m_borderOffset[1]) / bs - 1;
	float yTop = (m_borderThickness + m_borderOffset[0] + m_topOffset) / bs;
	float yBottom = m_maxY - (m_borderThickness + m_borderOffset[2]) / bs - 1;

	if ((l_snake.GetPosition().x < xLeft) || (l_snake.GetPosition().x > xRight) ||
		(l_snake.GetPosition().y < yTop) || (l_snake.GetPosition().y > yBottom))
	{
		l_snake.LoseStatus(true);
		Reset(l_window, l_snake);
	}
}

void World::Update(Window& l_window, Snake& l_snake)
{
	if (l_snake.GetPosition().x == m_applePos.x && l_snake.GetPosition().y == m_applePos.y)
	{
		if (m_shrinkInterval > 0)
		{
			if (++m_count >= m_shrinkInterval)
			{
				NarrowWorld(l_window, l_snake);
				m_count = 0;
			}
		}

		m_totalApplesEaten++;
		l_snake.Extend();
		RespawnApple(l_snake);
	}

	CheckCollision(l_window, l_snake);
}

void World::SetShrinkInterval(int l_interval)
{
	m_shrinkInterval = l_interval;
}

void World::SetShrinkTimerSec(float l_sec)
{
	m_shrinkTimerSec = l_sec;
}

void World::UpdateTimedShrink(float l_dt, Window& l_window, Snake& l_snake)
{
	if (m_shrinkTimerSec <= 0.0f) return;
	m_shrinkTimerAccum += l_dt;
	while (m_shrinkTimerAccum >= m_shrinkTimerSec)
	{
		TriggerShrink(l_window, l_snake);
		m_shrinkTimerAccum -= m_shrinkTimerSec;
	}
}

void World::TriggerShrink(Window& l_window, Snake& l_snake)
{
	NarrowWorld(l_window, l_snake);
}

void World::SetBorderOffset(int l_side, float l_offsetPixels)
{
	if (l_side >= 0 && l_side < 4)
		m_borderOffset[l_side] = l_offsetPixels;
}

float World::GetBorderOffset(int l_side) const
{
	return (l_side >= 0 && l_side < 4) ? m_borderOffset[l_side] : 0.0f;
}

void World::ResetBorderOffsets()
{
	for (int i = 0; i < 4; i++)
		m_borderOffset[i] = 0.0f;
}

float World::GetEffectiveThickness(int l_side) const
{
	if (l_side >= 0 && l_side < 4)
		return m_borderThickness + m_borderOffset[l_side];
	return m_borderThickness;
}

bool World::IsAppleInBounds(float l_blockSize) const
{
	float bs = l_blockSize;
	float xLeft = (m_borderThickness + m_borderOffset[3]) / bs;
	float xRight = m_maxX - (m_borderThickness + m_borderOffset[1]) / bs - 2 * (m_appleRaduis / bs);
	float yTop = (m_borderThickness + m_borderOffset[0] + m_topOffset) / bs;
	float yBottom = m_maxY - (m_borderThickness + m_borderOffset[2]) / bs - 2 * (m_appleRaduis / bs);

	return m_applePos.x >= xLeft && m_applePos.x <= xRight &&
		   m_applePos.y >= yTop && m_applePos.y <= yBottom;
}

void World::SetAppleColor(sf::Color l_color)
{
	m_apple.setFillColor(l_color);
}

void World::Render(Window& l_window)
{
	for (int i = 0; i < 4; i++)
		l_window.Draw(m_borders[i]);

	l_window.Draw(m_apple);
}