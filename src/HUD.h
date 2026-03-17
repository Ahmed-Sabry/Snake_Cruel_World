#pragma once

#include "Window.h"
#include "LevelConfig.h"
#include <string>

class HUD
{
public:
	HUD();
	~HUD() = default;

	void Update(int l_score, float l_combo, int l_applesEaten, int l_applesToWin,
				const std::string& l_levelName, float l_time, float l_dt);
	void Render(Window& l_window);
	void SetVisible(bool l_visible);

	// Flash score text when combo changes
	void FlashCombo();

private:
	sf::Font m_font;
	sf::Text m_scoreText;
	sf::Text m_comboText;
	sf::Text m_appleText;
	sf::Text m_levelText;
	sf::Text m_timerText;
	bool m_visible;
	float m_comboFlashTimer;
};
