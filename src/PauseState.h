#pragma once

#include "GameState.h"
#include "StateManager.h"

class PauseState : public BaseState
{
public:
	PauseState(StateManager& l_stateManager);
	~PauseState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_cruelTip;
	sf::Text m_menuItems[3]; // Resume, Restart, Quit to Menu
	sf::RectangleShape m_overlay;
	int m_selectedItem;
	int m_itemCount;
	bool m_keyReleased;

	static const char* s_cruelTips[];
	static const int s_tipCount;
};
