#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "PaperBackground.h"
#include <vector>

class AchievementState : public BaseState
{
public:
	AchievementState(StateManager& l_stateManager);
	~AchievementState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_counter;
	std::vector<sf::Text> m_lines;
	PaperBackground m_paperBg;
	bool m_keyReleased;
	int m_scrollOffset;
	int m_maxVisibleLines;
};
