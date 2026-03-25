#include "AbilityHUD.h"

#include "HUD.h"
#include "LevelConfig.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
	std::string FormatSeconds(float l_seconds)
	{
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(1) << l_seconds << "s";
		return ss.str();
	}
}

AbilityHUD::AbilityHUD(const sf::Vector2u& l_windowSize)
	: m_visible(true)
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "AbilityHUD: Failed to load font from " << FONT_PATH << std::endl;

	m_background.setSize({ 250.0f, 54.0f });
	m_background.setPosition((float)l_windowSize.x - 262.0f, HUD::HUD_HEIGHT + 8.0f);
	m_background.setFillColor(sf::Color(245, 235, 220, 210));
	m_background.setOutlineThickness(1.0f);
	m_background.setOutlineColor(sf::Color(60, 50, 45, 120));

	m_titleText.setFont(m_font);
	m_titleText.setCharacterSize(18);
	m_titleText.setPosition(m_background.getPosition().x + 10.0f, m_background.getPosition().y + 5.0f);

	m_statusText.setFont(m_font);
	m_statusText.setCharacterSize(15);
	m_statusText.setPosition(m_background.getPosition().x + 10.0f, m_background.getPosition().y + 28.0f);
}

void AbilityHUD::Update(const AbilityController& l_controller)
{
	const AbilityDefinition& equipped = l_controller.GetEquippedDefinition();
	m_titleText.setString("Ability: " + std::string(equipped.name));

	switch (l_controller.GetStatus(equipped.id))
	{
		case AbilityStatus::Locked:
			m_statusText.setString("Locked  |  Q cycle  |  Space use");
			break;
		case AbilityStatus::Ready:
			m_statusText.setString("Ready  |  Q cycle  |  Space use");
			break;
		case AbilityStatus::Active:
			m_statusText.setString("Active: " + FormatSeconds(l_controller.GetActiveRemaining()));
			break;
		case AbilityStatus::VisualOnly:
			m_statusText.setString("Active feedback");
			break;
		case AbilityStatus::Cooldown:
			m_statusText.setString("Cooldown: " + FormatSeconds(
				l_controller.GetCooldownRemaining(equipped.id)));
			break;
		default:
			m_statusText.setString("Locked  |  Q cycle  |  Space use");
			break;
	}
}

void AbilityHUD::Render(Window& l_window)
{
	if (!m_visible)
		return;

	l_window.Draw(m_background);
	l_window.Draw(m_titleText);
	l_window.Draw(m_statusText);
}

void AbilityHUD::SetVisible(bool l_visible)
{
	m_visible = l_visible;
}

void AbilityHUD::SetLevelColors(const sf::Color& l_paperTone, const sf::Color& l_inkTint, const sf::Color& l_accentColor)
{
	m_background.setFillColor(sf::Color(l_paperTone.r, l_paperTone.g, l_paperTone.b, 210));
	m_background.setOutlineColor(sf::Color(l_inkTint.r, l_inkTint.g, l_inkTint.b, 120));
	m_titleText.setFillColor(l_inkTint);
	m_statusText.setFillColor(sf::Color(
		std::min(255, (int)l_accentColor.r / 2 + 45),
		std::min(255, (int)l_accentColor.g / 2 + 45),
		std::min(255, (int)l_accentColor.b / 2 + 45)));
}
