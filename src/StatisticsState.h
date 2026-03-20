#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "PaperBackground.h"
#include <vector>

class StatisticsState : public BaseState
{
public:
	StatisticsState(StateManager& l_stateManager);
	~StatisticsState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	std::vector<sf::Text> m_lines;
	PaperBackground m_paperBg;
	bool m_keyReleased;
	int m_scrollOffset;
	int m_maxVisibleLines;
};
