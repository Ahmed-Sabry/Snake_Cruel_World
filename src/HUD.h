#pragma once

#include "Window.h"
#include "LevelConfig.h"
#include <string>
#include <algorithm>

class HUD
{
public:
	static constexpr float HUD_HEIGHT = 32.f;
	static constexpr float HUD_PADDING = 12.f;
	static constexpr float HUD_TEXT_Y = 5.f;

	HUD(const sf::Vector2u& l_windowSize);
	~HUD() = default;

	void Update(int l_score, float l_combo, int l_applesEaten, int l_applesToWin,
				const std::string& l_levelName, float l_time, float l_dt,
				int l_predatorApples = -1, int l_predatorMax = 5);
	void Render(Window& l_window);
	void SetVisible(bool l_visible);
	void SetLevelColors(const sf::Color& l_paperTone, const sf::Color& l_inkTint, const sf::Color& l_accentColor);

	static float GetHeight() { return HUD_HEIGHT; }

	// Flash score text when combo changes
	void FlashCombo();

private:
	sf::Font m_font;
	sf::Text m_scoreText;
	sf::Text m_comboText;
	sf::Text m_appleText;
	sf::Text m_levelText;
	sf::Text m_timerText;
	sf::Text m_predatorText;
	sf::RectangleShape m_background;
	sf::RectangleShape m_separator;
	unsigned int m_windowWidth;
	bool m_visible;
	float m_comboFlashTimer;
};
