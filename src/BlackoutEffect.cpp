#include "BlackoutEffect.h"

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

	// Dark overlay covers entire window
	sf::Vector2u winSize = l_window.GetWindowSize();
	m_overlay.setSize(sf::Vector2f((float)winSize.x, (float)winSize.y));
	m_overlay.setPosition(0.0f, 0.0f);
	m_overlay.setFillColor(sf::Color(0, 0, 0, 240));
	l_window.Draw(m_overlay);

	// Head glow: 3 concentric circles, innermost brightest
	for (int i = 2; i >= 0; i--)
	{
		float radius = l_blockSize * (1.0f + i * 1.5f);
		m_headGlow.setRadius(radius);
		m_headGlow.setOrigin(radius, radius);
		m_headGlow.setPosition(m_headPixelPos);
		int alpha = 25 - i * 8; // 25, 17, 9
		m_headGlow.setFillColor(sf::Color(255, 120, 80, alpha));
		l_window.Draw(m_headGlow);
	}
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
