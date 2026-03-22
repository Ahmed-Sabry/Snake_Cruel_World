#pragma once

#include "CutsceneTimeline.h"
#include <string>

class StateManager;

class CutsceneLoader
{
public:
	// Load a cutscene timeline from a JSON file or string.
	// Returns an empty CutsceneTimeline on missing file or parse error (logged to stderr).
	// Does not modify StateManager on failure.
	static CutsceneTimeline LoadFromFile(const std::string& l_path, StateManager& l_sm);
	static CutsceneTimeline LoadFromString(const std::string& l_json, StateManager& l_sm);

	struct CutsceneMetadata
	{
		std::string id;
		std::string displayName;
	};

	static bool ReadMetadata(const std::string& l_path, CutsceneMetadata& l_out);
};
