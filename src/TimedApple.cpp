#include "TimedApple.h"
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
	if (m_expired || m_timerSec <= 0.0f) return;

	float fraction = m_timeRemaining / m_timerSec;
	float maxRadius = l_appleRadius + 8.0f;
	float radius = maxRadius * fraction;

	if (radius < 1.0f) return;

	m_ring.setRadius(radius);
	m_ring.setOrigin(radius, radius);
	m_ring.setPosition(l_applePixelPos.x + l_appleRadius, l_applePixelPos.y + l_appleRadius);
	m_ring.setFillColor(sf::Color::Transparent);

	// Color based on urgency
	sf::Color ringColor;
	float pulse = (std::sin(m_timeRemaining * 8.0f) + 1.0f) / 2.0f;
	if (fraction > 0.5f)
		ringColor = sf::Color(255, 200, 0);
	else if (fraction > 0.25f)
		ringColor = sf::Color(255, 140, 0);
	else
		ringColor = sf::Color(255, 50, 30, (sf::Uint8)(180 + (int)(75.0f * pulse)));

	m_ring.setOutlineColor(ringColor);
	m_ring.setOutlineThickness(2.0f);
	l_window.Draw(m_ring);
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
