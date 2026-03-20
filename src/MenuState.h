#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "PaperBackground.h"

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
	sf::Text m_menuItems[3]; // Play, Level Select, Quit
	int m_selectedItem;
	int m_itemCount;
	bool m_keyReleased;

	float m_animTimer;
	PaperBackground m_paperBg;
};
