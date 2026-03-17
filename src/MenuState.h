#pragma once

#include "GameState.h"
#include "StateManager.h"

class MenuState : public BaseState
{
public:
	MenuState(StateManager& l_stateManager);
	~MenuState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_tagline;
	sf::Text m_menuItems[4]; // Play, Level Select, Settings, Quit
	int m_selectedItem;
	int m_itemCount;
	bool m_keyReleased;

	// Ouroboros animation
	float m_animTimer;
};
