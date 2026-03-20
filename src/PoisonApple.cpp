#include "PoisonApple.h"
#include "World.h"
#include "InkRenderer.h"
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

	sf::RenderTarget& target = l_window.GetRenderWindow();

	float baseRadius = l_blockSize / 2.0f;
	float cx = m_pos.x * l_blockSize + baseRadius;
	float cy = m_pos.y * l_blockSize + baseRadius;

	// Nervous wobble — higher frequency than real apple to be subtly different
	float wobbleRadius = baseRadius + std::sin(m_pulseTimer * 6.0f) * 1.2f;

	// Ink-style wobbly circle — looks nearly identical to real apple
	sf::Color poisonFill = sf::Color::Green;
	sf::Color outlineColor(20, 50, 20, 200);

	// Higher corruption = more nervous line (the "tell")
	float nervousCorruption = 0.35f;

	InkRenderer::DrawWobblyCircle(target, cx, cy, wobbleRadius,
								  poisonFill, outlineColor, 1.5f,
								  nervousCorruption,
								  (unsigned int)(m_pulseTimer * 3.0f), // Shifting seed = jittery
								  16);

	// Micro-drip effect every ~2 seconds
	float dripPhase = std::fmod(m_pulseTimer, 2.0f);
	if (dripPhase < 0.5f)
	{
		float dripAlpha = (0.5f - dripPhase) / 0.5f;
		float dripY = cy + wobbleRadius + dripPhase * 8.0f;
		sf::RectangleShape drip(sf::Vector2f(1.0f, 4.0f));
		drip.setPosition(cx, dripY);
		drip.setFillColor(sf::Color(20, 80, 20, (sf::Uint8)(180 * dripAlpha)));
		target.draw(drip);
	}

	// Phase 2 visual tell: faint crossed lines inside
	if (m_realApplesEaten >= 8)
	{
		sf::Color crossColor(20, 50, 20, 50);
		float cr = wobbleRadius * 0.5f;
		InkRenderer::DrawWobblyLine(target, cx - cr, cy - cr, cx + cr, cy + cr,
									crossColor, 0.8f, 0.15f, 901);
		InkRenderer::DrawWobblyLine(target, cx + cr, cy - cr, cx - cr, cy + cr,
									crossColor, 0.8f, 0.15f, 902);
	}
}

void PoisonApple::SpawnPoison(const Snake& l_snake, const World& l_world, float l_blockSize)
{
	m_pos = { -1, -1 }; // reset so stale position is never kept on failure

	float bs = l_blockSize;
	int xMin = (int)(l_world.GetEffectiveThickness(3) / bs);
	int xMax = (int)(l_world.GetMaxX() - l_world.GetEffectiveThickness(1) / bs - 1);
	int yMin = (int)((l_world.GetEffectiveThickness(0) + l_world.GetTopOffset()) / bs);
	int yMax = (int)(l_world.GetMaxY() - l_world.GetEffectiveThickness(2) / bs - 1);

	if (xMin > xMax) xMin = xMax;
	if (yMin > yMax) yMin = yMax;

	Position head = l_snake.GetPosition();
	sf::Vector2f applePos = l_world.GetApplePos();
	const auto& body = l_snake.GetBody();

	if (m_realApplesEaten < 8)
	{
		// Phase 1: spawn closer to snake than the real apple
		float realDist = std::abs(head.x - applePos.x) + std::abs(head.y - applePos.y);

		for (int attempt = 0; attempt < 100; attempt++)
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

			float poisonDist = std::abs(head.x - px) + std::abs(head.y - py);
			if (poisonDist < realDist)
			{
				m_pos = { (float)px, (float)py };
				return;
			}
		}

		// Fallback
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

		px = std::max(xMin, std::min(xMax, px));
		py = std::max(yMin, std::min(yMax, py));

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

	m_controlInverted = true;
	m_invertTimer = 4.0f;

	if (m_consecutivePoisons >= 2)
	{
		m_speedBoostAmount = (m_consecutivePoisons >= 3) ? 1.5f : 1.3f;
		m_speedBoostTimer = 6.0f;
	}

	if (m_consecutivePoisons >= 3)
		m_extraGrowth = 3;
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

bool PoisonApple::IsInBounds(const World& l_world, float l_blockSize) const
{
	if (m_pos.x < 0) return true;

	float bs = l_blockSize;
	float xMin = l_world.GetEffectiveThickness(3) / bs;
	float xMax = l_world.GetMaxX() - l_world.GetEffectiveThickness(1) / bs - 1;
	float yMin = (l_world.GetEffectiveThickness(0) + l_world.GetTopOffset()) / bs;
	float yMax = l_world.GetMaxY() - l_world.GetEffectiveThickness(2) / bs - 1;

	return m_pos.x >= xMin && m_pos.x <= xMax &&
		   m_pos.y >= yMin && m_pos.y <= yMax;
}
