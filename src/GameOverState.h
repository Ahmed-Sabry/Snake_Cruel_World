#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "ScreenShake.h"

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
	sf::Text m_menuItems[4]; // Next Level, Retry, Level Select, Main Menu
	int m_selectedItem;
	int m_itemCount;
	bool m_hasNextLevel;
	bool m_keyReleased;
	ScreenShake m_screenShake;

	static const char* s_deathTaunts[];
	static const int s_deathTauntCount;
	static const char* s_victoryTaunts[];
	static const int s_victoryTauntCount;
	static const char* s_fastDeathTaunts[];
	static const int s_fastDeathTauntCount;
};
