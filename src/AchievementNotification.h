#pragma once

#include "Window.h"
#include "AchievementDefs.h"
#include <queue>
#include <string>

// Non-intrusive in-game popup for achievement unlocks.
// Owned by PlayState, rendered after HUD.
class AchievementNotification
{
public:
	AchievementNotification();

	void Init(const sf::Font& l_font);
	void Push(const AchievementDef& l_def);
	void Update(float l_dt);
	void Render(sf::RenderTarget& l_target, float l_windowWidth);
	bool IsShowing() const;

private:
	enum class Phase { Idle, SlideIn, Hold, SlideOut };
	Phase m_phase;
	float m_timer;
	float m_slideInDuration;
	float m_holdDuration;
	float m_slideOutDuration;

	std::string m_currentName;
	std::string m_currentDesc;
	sf::Text m_nameText;
	sf::Text m_descText;
	bool m_fontLoaded;

	struct Pending { std::string name; std::string desc; };
	std::queue<Pending> m_queue;

	void StartNext();
};
