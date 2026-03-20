#include "BlackoutEffect.h"
#include <cmath>

BlackoutEffect::BlackoutEffect()
	: m_cycleTimer(0.0f),
	  m_lightDuration(10.0f),
	  m_darkDuration(3.0f),
	  m_isDark(false),
	  m_justStarted(false)
{
}

void BlackoutEffect::Reset()
{
	m_cycleTimer = 0.0f;
	m_isDark = false;
	m_justStarted = false;
}

void BlackoutEffect::Update(float l_dt, const Snake& l_snake)
{
	m_cycleTimer += l_dt;

	if (!m_isDark)
	{
		if (m_cycleTimer >= m_lightDuration)
		{
			m_isDark = true;
			m_justStarted = true;
			m_cycleTimer = 0.0f;
		}
	}
	else
	{
		if (m_cycleTimer >= m_darkDuration)
		{
			m_isDark = false;
			m_cycleTimer = 0.0f;
		}
	}

	// Track head position for glow rendering
	Position head = l_snake.GetPosition();
	float bs = l_snake.GetBlockSize();
	m_headPixelPos = sf::Vector2f(head.x * bs + bs / 2.0f, head.y * bs + bs / 2.0f);
}

void BlackoutEffect::Render(Window& l_window, float l_blockSize)
{
	if (!m_isDark) return;

	sf::Vector2u winSize = l_window.GetWindowSize();

	// Dark overlay — like ink spilled across the page
	m_overlay.setSize(sf::Vector2f((float)winSize.x, (float)winSize.y));
	m_overlay.setPosition(0.0f, 0.0f);
	m_overlay.setFillColor(sf::Color(30, 20, 15, 235)); // Dark sepia instead of pure black

	l_window.Draw(m_overlay);

	// Head glow: warm candlelight illuminating the notebook page
	// 5 concentric circles for smoother gradient, warm orange tint
	for (int i = 4; i >= 0; i--)
	{
		float radius = l_blockSize * (0.8f + i * 1.2f);
		m_headGlow.setRadius(radius);
		m_headGlow.setOrigin(radius, radius);
		m_headGlow.setPosition(m_headPixelPos);

		// Warm candlelight colors with noisy flicker
		float flicker = 1.0f + std::sin(m_cycleTimer * 12.0f + (float)i * 1.5f) * 0.15f;
		int baseAlpha = 30 - i * 5; // 30, 25, 20, 15, 10
		int alpha = (int)(baseAlpha * flicker);
		if (alpha < 0) alpha = 0;
		if (alpha > 255) alpha = 255;

		m_headGlow.setFillColor(sf::Color(255, 140, 60, (sf::Uint8)alpha));
		l_window.Draw(m_headGlow);
	}

	// Inner bright core
	float coreRadius = l_blockSize * 0.6f;
	m_headGlow.setRadius(coreRadius);
	m_headGlow.setOrigin(coreRadius, coreRadius);
	m_headGlow.setPosition(m_headPixelPos);
	m_headGlow.setFillColor(sf::Color(255, 200, 120, 35));
	l_window.Draw(m_headGlow);
}

bool BlackoutEffect::IsBlackout() const
{
	return m_isDark;
}

bool BlackoutEffect::JustStartedBlackout()
{
	if (m_justStarted)
	{
		m_justStarted = false;
		return true;
	}
	return false;
}
