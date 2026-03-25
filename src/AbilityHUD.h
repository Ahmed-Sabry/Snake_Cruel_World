#pragma once

#include "Ability.h"
#include "Window.h"

class AbilityHUD
{
public:
	AbilityHUD(const sf::Vector2u& l_windowSize);
	~AbilityHUD() = default;

	void Update(const AbilityController& l_controller);
	void Render(Window& l_window);
	void SetVisible(bool l_visible);
	void SetLevelColors(const sf::Color& l_paperTone, const sf::Color& l_inkTint, const sf::Color& l_accentColor);

private:
	sf::Font m_font;
	sf::RectangleShape m_background;
	sf::Text m_titleText;
	sf::Text m_statusText;
	bool m_visible;
};
