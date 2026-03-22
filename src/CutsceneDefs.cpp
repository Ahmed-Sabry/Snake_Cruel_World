#include "CutsceneDefs.h"
#include "CutsceneActions.h"
#include "CutsceneState.h"
#include "CutsceneLoader.h"
#include "StateManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>

static std::vector<CutsceneDefs::CutsceneEntry> s_cachedEntries;
static bool s_entriesCached = false;

std::vector<CutsceneDefs::CutsceneEntry> CutsceneDefs::GetAllEntries()
{
	if (s_entriesCached)
		return s_cachedEntries;

	std::vector<CutsceneEntry> entries = {
		{"intro", "The Cruel Beginning", [](const StateManager& sm) { return sm.introPlayed; }, ""}
	};

	// Scan content/cutscenes/ for JSON cutscene files
	namespace fs = std::filesystem;
	const std::string cutsceneDir = "content/cutscenes";
	std::error_code ec;
	if (fs::exists(cutsceneDir, ec) && fs::is_directory(cutsceneDir, ec))
	{
		for (auto it = fs::directory_iterator(cutsceneDir, ec); it != fs::directory_iterator(); it.increment(ec))
		{
			if (ec)
			{
				std::cerr << "CutsceneDefs: directory iteration error: " << ec.message() << "\n";
				break;
			}
			if (it->path().extension() == ".json")
			{
				CutsceneLoader::CutsceneMetadata meta;
				if (CutsceneLoader::ReadMetadata(it->path().string(), meta))
				{
					// Check for duplicate id
					bool exists = false;
					for (auto& e : entries)
					{
						if (e.id == meta.id)
						{
							if (e.path.empty())
							{
								// Fill in path for preseeded entries
								e.path = it->path().string();
							}
							else
							{
								std::cerr << "CutsceneDefs: duplicate cutscene id '"
										  << meta.id << "' in " << it->path().string()
										  << " (already loaded from " << e.path << ")\n";
							}
							exists = true;
							break;
						}
					}
					if (!exists)
					{
						entries.push_back({meta.id, meta.displayName,
							[](const StateManager&) { return true; },
							it->path().string()});
					}
				}
			}
		}
	}

	s_cachedEntries = entries;
	s_entriesCached = true;
	return entries;
}

void CutsceneDefs::InvalidateCache()
{
	s_entriesCached = false;
	s_cachedEntries.clear();
}

CutsceneTimeline CutsceneDefs::Build(const std::string& l_id, StateManager& l_sm)
{
	// Resolve JSON path: check entries for a stored path, otherwise reconstruct
	std::string jsonPath;
	for (const auto& e : GetAllEntries())
	{
		if (e.id == l_id && !e.path.empty())
		{
			jsonPath = e.path;
			break;
		}
	}
	if (jsonPath.empty())
		jsonPath = "content/cutscenes/" + l_id + ".json";

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
	// Still set flags for known cutscenes so they don't re-queue
	if (l_id == "intro")
	{
		CutsceneTimeline tl;
		tl.Add(std::make_unique<LambdaAction>([](StateManager& sm)
		{
			sm.introPlayed = true;
		}));
		return tl;
	}
	return CutsceneTimeline{};
}
