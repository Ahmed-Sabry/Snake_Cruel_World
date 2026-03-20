#include "HUD.h"
#include "LevelConfig.h"
#include <sstream>
#include <iomanip>
#include <iostream>

HUD::HUD(const sf::Vector2u& l_windowSize) : m_visible(true), m_comboFlashTimer(0.0f)
{
	m_windowWidth = l_windowSize.x;

	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "HUD: Failed to load font from " << FONT_PATH << std::endl;

	// HUD background: semi-transparent to let paper show through
	m_background.setSize({(float)l_windowSize.x, HUD_HEIGHT});
	m_background.setPosition(0.f, 0.f);
	m_background.setFillColor(sf::Color(245, 235, 220, 200)); // Paper tone overlay

	// Separator: ink ruled line style
	m_separator.setSize({(float)l_windowSize.x, 3.f});
	m_separator.setPosition(0.f, HUD_HEIGHT - 3.f);
	m_separator.setFillColor(sf::Color(60, 50, 45, 100)); // Ink tint

	auto setupText = [this](sf::Text& text, int size, sf::Color color, sf::Vector2f pos)
	{
		text.setFont(m_font);
		text.setCharacterSize(size);
		text.setFillColor(color);
		text.setPosition(pos);
	};

	// Default ink-toned text colors (updated per level via SetLevelColors)
	sf::Color inkDark(45, 40, 55);
	sf::Color inkAccent(170, 65, 55);
	sf::Color inkGreen(55, 100, 50);

	setupText(m_scoreText, 20, inkDark, { HUD_PADDING, HUD_TEXT_Y });
	setupText(m_comboText, 18, inkAccent, { 200.f, HUD_TEXT_Y + 1.f });
	setupText(m_levelText, 18, sf::Color(100, 90, 80), { 0.f, HUD_TEXT_Y });
	setupText(m_appleText, 18, inkGreen, { 0.f, HUD_TEXT_Y });
	setupText(m_timerText, 18, sf::Color(100, 90, 80), { 0.f, HUD_TEXT_Y });
	setupText(m_predatorText, 18, sf::Color(60, 80, 140), { 0.f, HUD_TEXT_Y });
}

void HUD::Update(int l_score, float l_combo, int l_applesEaten, int l_applesToWin,
				  const std::string& l_levelName, float l_time, float l_dt,
				  int l_predatorApples, int l_predatorMax)
{
	// Score (left-anchored)
	m_scoreText.setString("Score: " + std::to_string(l_score));

	// Combo (positioned dynamically after score)
	if (l_combo > 1.0f)
	{
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(1) << "x" << l_combo;
		m_comboText.setString(ss.str());

		if (l_combo >= 3.0f)
			m_comboText.setFillColor(sf::Color(200, 170, 40)); // gold ink
		else
			m_comboText.setFillColor(sf::Color(190, 160, 45));
	}
	else
	{
		m_comboText.setString("");
	}

	float comboX = m_scoreText.getPosition().x + m_scoreText.getGlobalBounds().width + 15.f;
	m_comboText.setPosition(comboX, HUD_TEXT_Y + 1.f);

	// Level name (centered)
	m_levelText.setString(l_levelName);
	sf::FloatRect levelBounds = m_levelText.getGlobalBounds();
	m_levelText.setPosition((m_windowWidth - levelBounds.width) / 2.f, HUD_TEXT_Y);

	// Timer (right-anchored)
	int mins = (int)l_time / 60;
	int secs = (int)l_time % 60;
	std::ostringstream ts;
	ts << std::setw(2) << std::setfill('0') << mins << ":" << std::setw(2) << std::setfill('0') << secs;
	m_timerText.setString(ts.str());

	sf::FloatRect timerBounds = m_timerText.getGlobalBounds();
	m_timerText.setPosition(m_windowWidth - timerBounds.width - HUD_PADDING, HUD_TEXT_Y);

	// Predator counter (positioned left of timer, only when active)
	float predatorRightEdge = m_timerText.getPosition().x;
	if (l_predatorApples >= 0)
	{
		m_predatorText.setString("Predator: " + std::to_string(l_predatorApples) + "/" + std::to_string(l_predatorMax));

		// Color urgency: ink blue -> amber -> rust red
		if (l_predatorApples >= l_predatorMax - 1)
			m_predatorText.setFillColor(sf::Color(180, 45, 30));
		else if (l_predatorApples >= l_predatorMax - 2)
			m_predatorText.setFillColor(sf::Color(190, 120, 40));
		else
			m_predatorText.setFillColor(sf::Color(70, 100, 160));

		sf::FloatRect predBounds = m_predatorText.getGlobalBounds();
		float predX = m_timerText.getPosition().x - predBounds.width - 25.f;
		m_predatorText.setPosition(predX, HUD_TEXT_Y);
		predatorRightEdge = predX;
	}
	else
	{
		m_predatorText.setString("");
	}

	// Apples (positioned left of predator counter or timer)
	if (l_applesToWin > 0)
		m_appleText.setString("Apples: " + std::to_string(l_applesEaten) + "/" + std::to_string(l_applesToWin));
	else
		m_appleText.setString("Apples: " + std::to_string(l_applesEaten));
	sf::FloatRect appleBounds = m_appleText.getGlobalBounds();
	float applesX = predatorRightEdge - appleBounds.width - 25.f;
	m_appleText.setPosition(applesX, HUD_TEXT_Y);

	// Combo flash effect
	if (m_comboFlashTimer > 0.0f)
	{
		m_comboFlashTimer -= l_dt;
		if (m_comboFlashTimer < 0.0f)
			m_comboFlashTimer = 0.0f;
		int alpha = (int)(255 * (m_comboFlashTimer / 0.5f));
		m_comboText.setFillColor(sf::Color(210, 190, 60, std::min(255, alpha + 150)));
	}
}

void HUD::Render(Window& l_window)
{
	if (!m_visible)
		return;

	l_window.Draw(m_background);
	l_window.Draw(m_separator);
	l_window.Draw(m_scoreText);
	l_window.Draw(m_comboText);
	l_window.Draw(m_levelText);
	l_window.Draw(m_appleText);
	l_window.Draw(m_predatorText);
	l_window.Draw(m_timerText);
}

void HUD::SetVisible(bool l_visible)
{
	m_visible = l_visible;
}

void HUD::SetLevelColors(const sf::Color& l_paperTone, const sf::Color& l_inkTint, const sf::Color& l_accentColor)
{
	// Derive HUD colors from the level's paper and ink tones
	m_background.setFillColor(sf::Color(l_paperTone.r, l_paperTone.g, l_paperTone.b, 200));
	m_separator.setFillColor(sf::Color(l_inkTint.r, l_inkTint.g, l_inkTint.b, 100));

	// Score text in full ink
	m_scoreText.setFillColor(sf::Color(l_inkTint.r, l_inkTint.g, l_inkTint.b));
	// Level name / timer in lighter ink
	sf::Color lightInk(std::min(255, (int)l_inkTint.r + 40),
					   std::min(255, (int)l_inkTint.g + 40),
					   std::min(255, (int)l_inkTint.b + 35));
	m_levelText.setFillColor(lightInk);
	m_timerText.setFillColor(lightInk);
	// Apple counter in accent-derived color
	m_appleText.setFillColor(sf::Color(
		std::min(255, (int)l_accentColor.r / 2 + 20),
		std::min(255, (int)l_accentColor.g / 2 + 30),
		std::min(255, (int)l_accentColor.b / 2 + 20)));
}

void HUD::FlashCombo()
{
	m_comboFlashTimer = 0.5f;
}
