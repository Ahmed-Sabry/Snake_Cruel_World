#include "HUD.h"
#include "LevelConfig.h"
#include <sstream>
#include <iomanip>
#include <iostream>

HUD::HUD() : m_visible(true), m_comboFlashTimer(0.0f)
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "HUD: Failed to load font from " << FONT_PATH << std::endl;

	auto setupText = [this](sf::Text& text, int size, sf::Color color, sf::Vector2f pos)
	{
		text.setFont(m_font);
		text.setCharacterSize(size);
		text.setFillColor(color);
		text.setPosition(pos);
	};

	setupText(m_scoreText, 20, sf::Color::White, { 20.f, 8.f });
	setupText(m_comboText, 18, sf::Color(255, 200, 50), { 200.f, 10.f });
	setupText(m_levelText, 18, sf::Color(200, 200, 200), { 580.f, 8.f });
	setupText(m_appleText, 18, sf::Color(100, 255, 100), { 1150.f, 8.f });
	setupText(m_timerText, 18, sf::Color(200, 200, 200), { 1250.f, 8.f });
}

void HUD::Update(int l_score, float l_combo, int l_applesEaten, int l_applesToWin,
				  const std::string& l_levelName, float l_time, float l_dt)
{
	m_scoreText.setString("Score: " + std::to_string(l_score));

	if (l_combo > 1.0f)
	{
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(1) << "x" << l_combo;
		m_comboText.setString(ss.str());

		if (l_combo >= 3.0f)
			m_comboText.setFillColor(sf::Color(255, 215, 0)); // gold
		else
			m_comboText.setFillColor(sf::Color(255, 200, 50));
	}
	else
	{
		m_comboText.setString("");
	}

	m_levelText.setString(l_levelName);

	m_appleText.setString("Apples: " + std::to_string(l_applesEaten) + "/" + std::to_string(l_applesToWin));

	int mins = (int)l_time / 60;
	int secs = (int)l_time % 60;
	std::ostringstream ts;
	ts << mins << ":" << std::setw(2) << std::setfill('0') << secs;
	m_timerText.setString(ts.str());

	if (m_comboFlashTimer > 0.0f)
	{
		m_comboFlashTimer -= l_dt;
		int alpha = (int)(255 * (m_comboFlashTimer / 0.5f));
		m_comboText.setFillColor(sf::Color(255, 255, 100, std::min(255, alpha + 150)));
	}
}

void HUD::Render(Window& l_window)
{
	if (!m_visible)
		return;

	l_window.Draw(m_scoreText);
	l_window.Draw(m_comboText);
	l_window.Draw(m_levelText);
	l_window.Draw(m_appleText);
	l_window.Draw(m_timerText);
}

void HUD::SetVisible(bool l_visible)
{
	m_visible = l_visible;
}

void HUD::FlashCombo()
{
	m_comboFlashTimer = 0.5f;
}
