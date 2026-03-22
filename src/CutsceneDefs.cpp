#include "CutsceneDefs.h"
#include "CutsceneActions.h"
#include "CutsceneState.h"
#include "CutsceneLoader.h"
#include "StateManager.h"
#include <fstream>
#include <filesystem>

std::vector<CutsceneDefs::CutsceneEntry> CutsceneDefs::GetAllEntries()
{
	std::vector<CutsceneEntry> entries = {
		{"intro", "The Cruel Beginning", [](const StateManager& sm) { return sm.introPlayed; }}
	};

	// Scan content/cutscenes/ for JSON cutscene files
	namespace fs = std::filesystem;
	const std::string cutsceneDir = "content/cutscenes";
	if (fs::exists(cutsceneDir) && fs::is_directory(cutsceneDir))
	{
		for (const auto& entry : fs::directory_iterator(cutsceneDir))
		{
			if (entry.path().extension() == ".json")
			{
				CutsceneLoader::CutsceneMetadata meta;
				if (CutsceneLoader::ReadMetadata(entry.path().string(), meta))
				{
					// Skip if already in hardcoded list
					bool exists = false;
					for (const auto& e : entries)
					{
						if (e.id == meta.id)
						{
							exists = true;
							break;
						}
					}
					if (!exists)
					{
						entries.push_back({meta.id, meta.displayName,
							[](const StateManager&) { return true; }});
					}
				}
			}
		}
	}

	return entries;
}

CutsceneTimeline CutsceneDefs::Build(const std::string& l_id, StateManager& l_sm)
{
	// Load from JSON cutscene file
	std::string jsonPath = "content/cutscenes/" + l_id + ".json";
	{
		std::ifstream test(jsonPath);
		if (test.good())
		{
			CutsceneTimeline tl = CutsceneLoader::LoadFromFile(jsonPath, l_sm);

			// Append cutscene-specific C++ logic that can't be expressed in JSON
			if (l_id == "intro")
			{
				tl.Add(std::make_unique<LambdaAction>([](StateManager& sm)
				{
					sm.introPlayed = true;
				}));
			}

			return tl;
		}
	}

	// Unknown cutscene ID — return empty timeline (instantly finishes)
	return CutsceneTimeline{};
}
