#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "PaperBackground.h"
#include "SnakeSkin.h"
#include <vector>

class SkinSelectState : public BaseState
{
public:
	SkinSelectState(StateManager& l_stateManager);
	~SkinSelectState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	bool IsSkinUnlocked(int l_skinIndex) const;
	sf::Color HsvToRgb(float h, float s, float v) const;

	sf::Font m_font;
	sf::Text m_title;
	std::vector<sf::Text> m_skinLabels;
	std::vector<SnakeSkin> m_skins;
	int m_selectedItem;
	bool m_keyReleased;
	float m_animTimer;
	PaperBackground m_paperBg;
};
