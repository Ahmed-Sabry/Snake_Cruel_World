#include "TimedApple.h"
#include "InkRenderer.h"
#include <cmath>
#include <algorithm>

TimedApple::TimedApple()
	: m_timerSec(5.0f),
	  m_timeRemaining(5.0f),
	  m_expired(false)
{
}

void TimedApple::Reset(float l_timerSec)
{
	m_timerSec = l_timerSec;
	m_timeRemaining = l_timerSec;
	m_expired = false;
}

void TimedApple::Update(float l_dt)
{
	if (m_expired) return;

	m_timeRemaining -= l_dt;
	if (m_timeRemaining <= 0.0f)
	{
		m_timeRemaining = 0.0f;
		m_expired = true;
	}
}

void TimedApple::Render(Window& l_window, sf::Vector2f l_applePixelPos, float l_appleRadius)
{
	RenderTo(l_window.GetRenderWindow(), l_applePixelPos, l_appleRadius);
}

void TimedApple::RenderTo(sf::RenderTarget& target, sf::Vector2f l_applePixelPos, float l_appleRadius)
{
	if (m_expired || m_timerSec <= 0.0f) return;

	float fraction = m_timeRemaining / m_timerSec;
	float maxRadius = l_appleRadius + 8.0f;

	// Color based on urgency — ink-toned
	sf::Color ringColor;
	if (fraction > 0.5f)
		ringColor = sf::Color(140, 110, 30); // Warm sepia
	else if (fraction > 0.25f)
		ringColor = sf::Color(180, 80, 20);  // Rust
	else
	{
		// Trembling red — pulse alpha
		float pulse = (std::sin(m_timeRemaining * 8.0f) + 1.0f) / 2.0f;
		ringColor = sf::Color(200, 40, 20, (sf::Uint8)(160 + (int)(95.0f * pulse)));
	}

	// Dashed arc ring that "erases" as time depletes
	float cx = l_applePixelPos.x + l_appleRadius;
	float cy = l_applePixelPos.y + l_appleRadius;

	float corruption = (fraction < 0.25f) ? 0.3f : 0.1f;
	unsigned int seed = (unsigned int)(m_timeRemaining * 100.0f);

	InkRenderer::DrawDashedArc(target, cx, cy, maxRadius,
							   fraction, ringColor, 2.0f,
							   corruption, seed);
}

bool TimedApple::HasExpired() const
{
	return m_expired;
}

void TimedApple::OnAppleEaten(float l_newTimerSec)
{
	m_timerSec = l_newTimerSec;
	m_timeRemaining = l_newTimerSec;
	m_expired = false;
}

float TimedApple::GetTimeRemaining() const
{
	return m_timeRemaining;
}

float TimedApple::GetTimerDuration() const
{
	return m_timerSec;
}
