#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "ScreenShake.h"
#include <vector>
#include <string>

enum class GameOverAction { StageSelect, Retry, MainMenu };

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
	std::string SelectDeathTaunt(int l_levelId, int l_applesToWin);
	std::string SelectVictoryTaunt(int l_levelId, int l_stars);

	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_taunt;
	sf::Text m_stats[7]; // score, apples, combo, time, collisions, death#, best attempt
	int m_statCount;
	sf::Text m_menuItems[3]; // Stage Select (optional), Retry, Main Menu
	std::vector<GameOverAction> m_menuActions;
	int m_selectedItem;
	int m_itemCount;
	bool m_keyReleased;
	ScreenShake m_screenShake;

	static const char* s_deathTaunts[];
	static const int s_deathTauntCount;
	static const char* s_victoryTaunts[];
	static const int s_victoryTauntCount;
	static const char* s_fastDeathTaunts[];
	static const int s_fastDeathTauntCount;
};
