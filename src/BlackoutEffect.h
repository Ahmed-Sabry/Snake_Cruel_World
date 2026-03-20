#pragma once

#include "Window.h"
#include "Snake.h"

class BlackoutEffect
{
public:
	BlackoutEffect();

	void Reset();
	void Update(float l_dt, const Snake& l_snake);
	void Render(Window& l_window, float l_blockSize);
	void RenderTo(sf::RenderTarget& l_target, sf::Vector2u l_winSize, float l_blockSize);
	bool IsBlackout() const;
	bool JustStartedBlackout();

private:
	float m_cycleTimer;
	float m_lightDuration;
	float m_darkDuration;
	bool m_isDark;
	bool m_justStarted;

	sf::RectangleShape m_overlay;
	sf::CircleShape m_headGlow;
	sf::Vector2f m_headPixelPos;
};
