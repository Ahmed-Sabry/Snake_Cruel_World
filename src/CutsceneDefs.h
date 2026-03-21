#pragma once

#include "CutsceneTimeline.h"
#include <string>

class StateManager;

namespace CutsceneDefs
{
	CutsceneTimeline Build(const std::string& l_id, StateManager& l_sm);
}
