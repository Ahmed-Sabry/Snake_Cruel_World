#pragma once

#include "CutsceneTimeline.h"
#include <string>

class StateManager;

class CutsceneLoader
{
public:
	static CutsceneTimeline LoadFromFile(const std::string& l_path, StateManager& l_sm);
	static CutsceneTimeline LoadFromString(const std::string& l_json, StateManager& l_sm);

	struct CutsceneMetadata
	{
		std::string id;
		std::string displayName;
	};

	static bool ReadMetadata(const std::string& l_path, CutsceneMetadata& l_out);
};
