#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "PaperBackground.h"
#include <vector>
#include <string>

class CutsceneGalleryState : public BaseState
{
public:
	CutsceneGalleryState(StateManager& l_stateManager);
	~CutsceneGalleryState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_backHint;

	struct GalleryItem
	{
		std::string id;
		std::string label;
		sf::Text text;
		bool unlocked;
	};
	std::vector<GalleryItem> m_items;
	int m_selectedItem;
	bool m_keyReleased;

	float m_animTimer;
	PaperBackground m_paperBg;
};
