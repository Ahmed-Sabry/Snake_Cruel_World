#pragma once

#include "GameState.h"
#include "StateManager.h"

class GameOverState : public BaseState
{
public:
	GameOverState(StateManager& l_stateManager);
	~GameOverState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_taunt;
	sf::Text m_stats[5]; // score, apples, combo, time, collisions
	sf::Text m_menuItems[3]; // Retry, Level Select, Main Menu
	int m_selectedItem;
	int m_itemCount;
	bool m_keyReleased;
	float m_shakeTimer;
	float m_shakeOffsetX;
	float m_shakeOffsetY;

	static const char* s_deathTaunts[];
	static const int s_deathTauntCount;
	static const char* s_victoryTaunts[];
	static const int s_victoryTauntCount;
	static const char* s_fastDeathTaunts[];
	static const int s_fastDeathTauntCount;
};
