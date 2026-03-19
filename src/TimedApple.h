#pragma once

#include "Window.h"

class TimedApple
{
public:
	TimedApple();

	void Reset(float l_timerSec);
	void Update(float l_dt);
	void Render(Window& l_window, sf::Vector2f l_applePixelPos, float l_appleRadius);

	bool HasExpired() const;
	void OnAppleEaten(float l_newTimerSec);
	float GetTimeRemaining() const;
	float GetTimerDuration() const;

private:
	float m_timerSec;
	float m_timeRemaining;
	bool m_expired;

	sf::CircleShape m_ring;
};
