#pragma once

#include <string>
#include <fstream>

class StateManager;
class StatsManager;
class AchievementManager;

class SaveManager
{
public:
	static void Save(const StateManager& l_state, const StatsManager& l_stats,
					 const AchievementManager& l_achievements);
	static void Load(StateManager& l_state, StatsManager& l_stats,
					 AchievementManager& l_achievements);

private:
	static void RebuildCampaignProgressFromLegacyState(StateManager& l_state);

	static const std::string s_saveFile;
};
