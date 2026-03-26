#pragma once

#include "GameState.h"
#include "LevelConfig.h"
#include "PaperBackground.h"
#include <array>
#include <vector>

class StageSelectState : public BaseState
{
public:
	StageSelectState(StateManager& l_stateManager);
	~StageSelectState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	struct StageTile
	{
		int levelId = 0;
		sf::FloatRect bounds;
	};

	void BuildLayout();
	int GetSelectedLevelId() const;
	void MoveSelectionHorizontal(int l_direction);
	void MoveSelectionVertical(int l_direction);
	void ActivateSelection();
	void DrawStageTile(Window& l_window, const StageTile& l_tile, bool l_selected);
	void DrawFinaleGate(Window& l_window, bool l_selected);
	void DrawSelectionDetails(Window& l_window);
	void DrawStars(sf::RenderTarget& l_target, float l_x, float l_y,
		int l_stars, const sf::Color& l_outlineColor, unsigned int l_seed) const;
	sf::Color GetTileOutlineColor(bool l_selected, bool l_available, bool l_healed) const;
	sf::Color GetSelectionPulseColor(bool l_available) const;

	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_subtitle;
	sf::Text m_footer;
	sf::Text m_statusStrip;
	std::vector<StageTile> m_tiles;
	sf::FloatRect m_finaleBounds;
	sf::FloatRect m_detailBounds;
	PaperBackground m_paperBg;
	float m_animTimer;
	int m_selectedIndex;
	int m_lastGridSelection;
	bool m_keyReleased;
};
