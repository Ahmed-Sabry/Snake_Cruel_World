#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "LevelConfig.h"

class LevelSelectState : public BaseState
{
public:
	LevelSelectState(StateManager& l_stateManager);
	~LevelSelectState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_levelNames[NUM_LEVELS];
	sf::Text m_levelSubtitles[NUM_LEVELS];
	sf::Text m_levelStars[NUM_LEVELS];
	sf::Text m_levelScores[NUM_LEVELS];
	sf::Text m_backHint;
	int m_selectedLevel;
	bool m_keyReleased;
	float m_animTimer;
	std::array<LevelConfig, NUM_LEVELS> m_levels;
};
