#pragma once

#include "CutsceneAction.h"
#include <vector>

// Helper to build a vector of unique_ptrs (can't use initializer_list with move-only types)
template <typename... Args>
std::vector<CutsceneActionPtr> Actions(Args&&... args)
{
	std::vector<CutsceneActionPtr> vec;
	vec.reserve(sizeof...(args));
	(vec.push_back(std::forward<Args>(args)), ...);
	return vec;
}

class CutsceneTimeline
{
public:
	void Add(CutsceneActionPtr l_action);
	void AddParallel(std::vector<CutsceneActionPtr> l_actions);

	void Start(StateManager& l_sm);
	bool Update(float l_dt, StateManager& l_sm);
	void Render(sf::RenderTarget& l_target);
	void SkipCurrent();
	void ClearPersistentText();
	bool IsFinished() const { return m_finished; }

private:
	std::vector<CutsceneActionPtr> m_steps;
	size_t m_currentStep = 0;
	bool m_finished = false;
};
