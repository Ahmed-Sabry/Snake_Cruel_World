#pragma once

#include "CutsceneTimeline.h"
#include <string>
#include <vector>
#include <functional>

class StateManager;

namespace CutsceneDefs
{
	struct CutsceneEntry
	{
		std::string id;
		std::string displayName;
		std::function<bool(const StateManager&)> isUnlocked;
	};

	CutsceneTimeline Build(const std::string& l_id, StateManager& l_sm);
	std::vector<CutsceneEntry> GetAllEntries();
}
