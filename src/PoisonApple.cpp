#include "PoisonApple.h"
#include "World.h"
#include "RandomUtils.h"
#include <cmath>
#include <algorithm>

PoisonApple::PoisonApple()
	: m_pos(-1, -1),
	  m_controlInverted(false),
	  m_invertTimer(0.0f),
	  m_speedBoostTimer(0.0f),
	  m_speedBoostAmount(1.0f),
	  m_consecutivePoisons(0),
	  m_realApplesEaten(0),
	  m_pulseTimer(0.0f),
	  m_extraGrowth(0)
{
}

void PoisonApple::Reset(float l_blockSize)
{
	m_pos = { -1, -1 };
	m_controlInverted = false;
	m_invertTimer = 0.0f;
	m_speedBoostTimer = 0.0f;
	m_speedBoostAmount = 1.0f;
	m_consecutivePoisons = 0;
	m_realApplesEaten = 0;
	m_pulseTimer = 0.0f;
	m_extraGrowth = 0;

	float radius = l_blockSize / 2.0f;
	m_shape.setRadius(radius);
	m_shape.setFillColor(sf::Color::Green);
}

void PoisonApple::Update(float l_dt)
{
	m_pulseTimer += l_dt;

	if (m_invertTimer > 0.0f)
	{
		m_invertTimer -= l_dt;
		if (m_invertTimer <= 0.0f)
		{
			m_invertTimer = 0.0f;
			m_controlInverted = false;
		}
	}

	if (m_speedBoostTimer > 0.0f)
	{
		m_speedBoostTimer -= l_dt;
		if (m_speedBoostTimer <= 0.0f)
		{
			m_speedBoostTimer = 0.0f;
			m_speedBoostAmount = 1.0f;
		}
	}
}

void PoisonApple::Render(Window& l_window, float l_blockSize)
{
	if (m_pos.x < 0) return;

	float baseRadius = l_blockSize / 2.0f;
	float radius = baseRadius + std::sin(m_pulseTimer * 4.0f) * 1.0f;

	m_shape.setRadius(radius);
	m_shape.setOrigin(radius - baseRadius, radius - baseRadius);
	m_shape.setFillColor(sf::Color::Green);
	m_shape.setPosition(m_pos.x * l_blockSize, m_pos.y * l_blockSize);
	l_window.Draw(m_shape);
}

void PoisonApple::SpawnPoison(const Snake& l_snake, const World& l_world, float l_blockSize)
{
	m_pos = { -1, -1 }; // reset so stale position is never kept on failure

	float bs = l_blockSize;
	int xMin = (int)(l_world.GetBorderThickness() / bs);
	int xMax = (int)(l_world.GetMaxX() - l_world.GetBorderThickness() / bs - 1);
	int yMin = (int)((l_world.GetBorderThickness() + l_world.GetTopOffset()) / bs);
	int yMax = (int)(l_world.GetMaxY() - l_world.GetBorderThickness() / bs - 1);

	if (xMin > xMax) xMin = xMax;
	if (yMin > yMax) yMin = yMax;

	Position head = l_snake.GetPosition();
	sf::Vector2f applePos = l_world.GetApplePos();
	const auto& body = l_snake.GetBody();

	if (m_realApplesEaten < 8)
	{
		// Phase 1: spawn closer to snake than the real apple
		float realDist = std::abs(head.x - applePos.x) + std::abs(head.y - applePos.y);

		// Try up to 100 times to find a valid closer position
		for (int attempt = 0; attempt < 100; attempt++)
		{
			int px = RandomInt(xMin, xMax);
			int py = RandomInt(yMin, yMax);

			// Must not overlap real apple
			if (px == (int)applePos.x && py == (int)applePos.y) continue;

			// Must not overlap snake body
			bool onSnake = false;
			for (const auto& seg : body)
			{
				if (seg.x == px && seg.y == py) { onSnake = true; break; }
			}
			if (onSnake) continue;

			float poisonDist = std::abs(head.x - px) + std::abs(head.y - py);
			if (poisonDist < realDist)
			{
				m_pos = { (float)px, (float)py };
				return;
			}
		}

		// Fallback: just place it somewhere valid
		for (int attempt = 0; attempt < 50; attempt++)
		{
			int px = RandomInt(xMin, xMax);
			int py = RandomInt(yMin, yMax);
			if (px == (int)applePos.x && py == (int)applePos.y) continue;
			bool onSnake = false;
			for (const auto& seg : body)
			{
				if (seg.x == px && seg.y == py) { onSnake = true; break; }
			}
			if (onSnake) continue;
			m_pos = { (float)px, (float)py };
			return;
		}
	}
	else
	{
		// Phase 2: spawn in the snake's movement direction (4-8 tiles ahead)
		Direction dir = l_snake.GetDirection();
		int dx = 0, dy = 0;
		switch (dir)
		{
			case Direction::Up:    dy = -1; break;
			case Direction::Down:  dy = 1;  break;
			case Direction::Left:  dx = -1; break;
			case Direction::Right: dx = 1;  break;
			default: dx = 1; break;
		}

		int ahead = RandomInt(4, 8);
		int px = head.x + dx * ahead;
		int py = head.y + dy * ahead;

		// Clamp to playable bounds
		px = std::max(xMin, std::min(xMax, px));
		py = std::max(yMin, std::min(yMax, py));

		// If overlaps real apple or snake, offset randomly
		bool placed = false;
		for (int attempt = 0; attempt < 50; attempt++)
		{
			if (px == (int)applePos.x && py == (int)applePos.y)
			{
				px += RandomInt(-2, 2);
				py += RandomInt(-2, 2);
				px = std::max(xMin, std::min(xMax, px));
				py = std::max(yMin, std::min(yMax, py));
				continue;
			}
			bool onSnake = false;
			for (const auto& seg : body)
			{
				if (seg.x == px && seg.y == py) { onSnake = true; break; }
			}
			if (!onSnake) { placed = true; break; }
			px += RandomInt(-2, 2);
			py += RandomInt(-2, 2);
			px = std::max(xMin, std::min(xMax, px));
			py = std::max(yMin, std::min(yMax, py));
		}

		if (placed)
			m_pos = { (float)px, (float)py };
	}
}

bool PoisonApple::CheckCollision(const Position& l_snakeHead) const
{
	return l_snakeHead.x == (int)m_pos.x && l_snakeHead.y == (int)m_pos.y;
}

void PoisonApple::OnPoisonEaten()
{
	m_consecutivePoisons++;
	m_extraGrowth = 0;

	// Always: invert controls for 4 seconds
	m_controlInverted = true;
	m_invertTimer = 4.0f;

	if (m_consecutivePoisons >= 2)
	{
		// Speed boost
		m_speedBoostAmount = (m_consecutivePoisons >= 3) ? 1.5f : 1.3f;
		m_speedBoostTimer = 6.0f;
	}

	if (m_consecutivePoisons >= 3)
	{
		// Extra growth
		m_extraGrowth = 3;
	}
}

void PoisonApple::OnRealAppleEaten()
{
	m_consecutivePoisons = 0;
	m_realApplesEaten++;
}

bool PoisonApple::IsControlInverted() const
{
	return m_controlInverted;
}

float PoisonApple::GetSpeedMultiplier() const
{
	return m_speedBoostAmount;
}

int PoisonApple::GetGrowAmount() const
{
	return m_extraGrowth;
}

int PoisonApple::GetRealApplesEaten() const
{
	return m_realApplesEaten;
}

sf::Vector2f PoisonApple::GetPixelPos(float l_blockSize) const
{
	return { m_pos.x * l_blockSize, m_pos.y * l_blockSize };
}
