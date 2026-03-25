#include "SaveManager.h"
#include "Ability.h"
#include "LevelConfig.h"
#include "StateManager.h"
#include "StatsManager.h"
#include "AchievementManager.h"
#include "SnakeSkin.h"
#include <array>
#include <iostream>
#include <cstring>
#include <cstdint>

static_assert(sizeof(bool) == 1, "Save format assumes sizeof(bool) == 1");

const std::string SaveManager::s_saveFile = "save.dat";

void SaveManager::Save(const StateManager& l_state, const StatsManager& l_stats,
					   const AchievementManager& l_achievements)
{
	std::ofstream file(s_saveFile, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "SaveManager: Failed to open " << s_saveFile << " for writing" << std::endl;
		return;
	}

	// Version marker
	int version = 4;
	file.write(reinterpret_cast<const char*>(&version), sizeof(version));

	// === V1 block (backwards-compatible) ===
	file.write(reinterpret_cast<const char*>(&l_state.highestUnlockedLevel), sizeof(l_state.highestUnlockedLevel));
	file.write(reinterpret_cast<const char*>(l_state.highScores), sizeof(l_state.highScores));
	file.write(reinterpret_cast<const char*>(l_state.starRatings), sizeof(l_state.starRatings));
	file.write(reinterpret_cast<const char*>(&l_state.totalDeaths), sizeof(l_state.totalDeaths));

	// === V2 block ===
	const CumulativeStats& stats = l_stats.GetStats();
	file.write(reinterpret_cast<const char*>(&stats.totalApplesEaten), sizeof(stats.totalApplesEaten));
	file.write(reinterpret_cast<const char*>(&stats.totalSegmentsLost), sizeof(stats.totalSegmentsLost));
	file.write(reinterpret_cast<const char*>(&stats.totalPoisonApplesEaten), sizeof(stats.totalPoisonApplesEaten));
	file.write(reinterpret_cast<const char*>(&stats.totalPlaytimeSeconds), sizeof(stats.totalPlaytimeSeconds));
	file.write(reinterpret_cast<const char*>(&stats.bestCombo), sizeof(stats.bestCombo));
	file.write(reinterpret_cast<const char*>(&stats.bestSingleLevelScore), sizeof(stats.bestSingleLevelScore));
	file.write(reinterpret_cast<const char*>(stats.fastestLevelTimes), sizeof(stats.fastestLevelTimes));
	file.write(reinterpret_cast<const char*>(stats.deathsPerLevel), sizeof(stats.deathsPerLevel));
	file.write(reinterpret_cast<const char*>(stats.attemptsPerLevel), sizeof(stats.attemptsPerLevel));
	file.write(reinterpret_cast<const char*>(&stats.endlessBestScore), sizeof(stats.endlessBestScore));
	file.write(reinterpret_cast<const char*>(&stats.endlessBestTime), sizeof(stats.endlessBestTime));
	file.write(reinterpret_cast<const char*>(&stats.predatorApplesStolen), sizeof(stats.predatorApplesStolen));
	file.write(reinterpret_cast<const char*>(&stats.predatorKills), sizeof(stats.predatorKills));

	// Achievement unlock data
	const bool* unlockData = l_achievements.GetUnlockData();
	file.write(reinterpret_cast<const char*>(unlockData), NUM_ACHIEVEMENTS * sizeof(bool));

	// Active skin index
	file.write(reinterpret_cast<const char*>(&l_state.activeSkinIndex), sizeof(l_state.activeSkinIndex));

	// === V3 block ===
	uint8_t introFlag = l_state.introPlayed ? 1 : 0;
	file.write(reinterpret_cast<const char*>(&introFlag), sizeof(introFlag));

	// === V4 block ===
	std::array<uint8_t, ABILITY_COUNT> unlockedAbilityFlags{};
	for (std::size_t i = 0; i < ABILITY_COUNT; ++i)
		unlockedAbilityFlags[i] = l_state.unlockedAbilities[i] ? 1 : 0;
	file.write(reinterpret_cast<const char*>(unlockedAbilityFlags.data()),
			   unlockedAbilityFlags.size() * sizeof(uint8_t));
	int equippedAbility = static_cast<int>(l_state.equippedAbility);
	file.write(reinterpret_cast<const char*>(&equippedAbility), sizeof(equippedAbility));

	file.close();
}

void SaveManager::Load(StateManager& l_state, StatsManager& l_stats,
					   AchievementManager& l_achievements)
{
	std::ifstream file(s_saveFile, std::ios::binary);
	if (!file.is_open())
		return;

	int version = 0;
	file.read(reinterpret_cast<char*>(&version), sizeof(version));

	if (file.fail() || version < 1)
	{
		std::cerr << "SaveManager: Corrupted save file, ignoring." << std::endl;
		file.close();
		return;
	}

	// === V1 block ===
	file.read(reinterpret_cast<char*>(&l_state.highestUnlockedLevel), sizeof(l_state.highestUnlockedLevel));
	file.read(reinterpret_cast<char*>(l_state.highScores), sizeof(l_state.highScores));
	file.read(reinterpret_cast<char*>(l_state.starRatings), sizeof(l_state.starRatings));
	file.read(reinterpret_cast<char*>(&l_state.totalDeaths), sizeof(l_state.totalDeaths));

	if (file.fail())
	{
		std::cerr << "SaveManager: Error reading save file, resetting progress." << std::endl;
		l_state.highestUnlockedLevel = 1;
		l_state.totalDeaths = 0;
		for (int i = 0; i < NUM_LEVELS; i++)
		{
			l_state.highScores[i] = 0;
			l_state.starRatings[i] = 0;
		}
		file.close();
		return;
	}

	// Validate v1 data
	if (l_state.highestUnlockedLevel < 1 || l_state.highestUnlockedLevel > NUM_LEVELS)
		l_state.highestUnlockedLevel = 1;
	if (l_state.totalDeaths < 0)
		l_state.totalDeaths = 0;
	for (int i = 0; i < NUM_LEVELS; i++)
	{
		if (l_state.highScores[i] < 0)
			l_state.highScores[i] = 0;
		if (l_state.starRatings[i] < 0)
			l_state.starRatings[i] = 0;
		else if (l_state.starRatings[i] > 3)
			l_state.starRatings[i] = 3;
	}

	// === V2 block (only if version >= 2) ===
	if (version >= 2)
	{
		CumulativeStats& stats = l_stats.GetMutableStats();
		file.read(reinterpret_cast<char*>(&stats.totalApplesEaten), sizeof(stats.totalApplesEaten));
		file.read(reinterpret_cast<char*>(&stats.totalSegmentsLost), sizeof(stats.totalSegmentsLost));
		file.read(reinterpret_cast<char*>(&stats.totalPoisonApplesEaten), sizeof(stats.totalPoisonApplesEaten));
		file.read(reinterpret_cast<char*>(&stats.totalPlaytimeSeconds), sizeof(stats.totalPlaytimeSeconds));
		file.read(reinterpret_cast<char*>(&stats.bestCombo), sizeof(stats.bestCombo));
		file.read(reinterpret_cast<char*>(&stats.bestSingleLevelScore), sizeof(stats.bestSingleLevelScore));
		file.read(reinterpret_cast<char*>(stats.fastestLevelTimes), sizeof(stats.fastestLevelTimes));
		file.read(reinterpret_cast<char*>(stats.deathsPerLevel), sizeof(stats.deathsPerLevel));
		file.read(reinterpret_cast<char*>(stats.attemptsPerLevel), sizeof(stats.attemptsPerLevel));
		file.read(reinterpret_cast<char*>(&stats.endlessBestScore), sizeof(stats.endlessBestScore));
		file.read(reinterpret_cast<char*>(&stats.endlessBestTime), sizeof(stats.endlessBestTime));
		file.read(reinterpret_cast<char*>(&stats.predatorApplesStolen), sizeof(stats.predatorApplesStolen));
		file.read(reinterpret_cast<char*>(&stats.predatorKills), sizeof(stats.predatorKills));

		// Achievement unlock data
		bool unlockData[NUM_ACHIEVEMENTS] = {};
		file.read(reinterpret_cast<char*>(unlockData), NUM_ACHIEVEMENTS * sizeof(bool));
		if (!file.fail())
			l_achievements.SetUnlockData(unlockData);

		// Active skin index
		file.read(reinterpret_cast<char*>(&l_state.activeSkinIndex), sizeof(l_state.activeSkinIndex));

		// Validate v2 data
		if (file.fail())
		{
			std::cerr << "SaveManager: Error reading v2 data, keeping v1 progress." << std::endl;
			// Reset v2 state to safe defaults — partial reads may leave garbage
			stats = CumulativeStats{};
			l_state.activeSkinIndex = 0;
			file.close();
			return;
		}

		int numSkins = (int)GetAllSkins().size();
		if (l_state.activeSkinIndex < 0 || l_state.activeSkinIndex >= numSkins)
			l_state.activeSkinIndex = 0;

		// Clamp v2 stats to non-negative
		if (stats.totalApplesEaten < 0) stats.totalApplesEaten = 0;
		if (stats.totalSegmentsLost < 0) stats.totalSegmentsLost = 0;
		if (stats.totalPoisonApplesEaten < 0) stats.totalPoisonApplesEaten = 0;
		if (stats.totalPlaytimeSeconds < 0) stats.totalPlaytimeSeconds = 0;
		if (stats.bestCombo < 0) stats.bestCombo = 0;
		if (stats.bestSingleLevelScore < 0) stats.bestSingleLevelScore = 0;
		if (stats.endlessBestScore < 0) stats.endlessBestScore = 0;
		if (stats.endlessBestTime < 0.0f) stats.endlessBestTime = 0.0f;
		if (stats.predatorApplesStolen < 0) stats.predatorApplesStolen = 0;
		if (stats.predatorKills < 0) stats.predatorKills = 0;

		// Clamp per-level arrays to non-negative
		for (int i = 0; i < NUM_LEVELS; i++)
		{
			if (stats.fastestLevelTimes[i] < 0.0f) stats.fastestLevelTimes[i] = 0.0f;
			if (stats.deathsPerLevel[i] < 0) stats.deathsPerLevel[i] = 0;
			if (stats.attemptsPerLevel[i] < 0) stats.attemptsPerLevel[i] = 0;
		}
	}

	// === V3 block (only if version >= 3) ===
	if (version >= 3)
	{
		uint8_t introFlag = 0;
		file.read(reinterpret_cast<char*>(&introFlag), sizeof(introFlag));
		l_state.introPlayed = (file.fail() ? false : introFlag != 0);
	}

	// === V4 block (only if version >= 4) ===
	if (version >= 4)
	{
		std::array<uint8_t, ABILITY_COUNT> unlockedAbilityFlags{};
		file.read(reinterpret_cast<char*>(unlockedAbilityFlags.data()),
				  unlockedAbilityFlags.size() * sizeof(uint8_t));

		int equippedAbility = static_cast<int>(GetDefaultEquippedAbility());
		file.read(reinterpret_cast<char*>(&equippedAbility), sizeof(equippedAbility));
		if (file.fail())
		{
			std::cerr << "SaveManager: Error reading v4 data, resetting ability progress." << std::endl;
			for (bool& unlocked : l_state.unlockedAbilities)
				unlocked = false;
			l_state.equippedAbility = ResolveEquippedAbilityFromUnlocks(l_state.unlockedAbilities, AbilityId::None);
			file.close();
			return;
		}

		bool hadInvalidUnlockFlag = false;
		for (std::size_t i = 0; i < ABILITY_COUNT; ++i)
		{
			if (unlockedAbilityFlags[i] > 1)
			{
				hadInvalidUnlockFlag = true;
				l_state.unlockedAbilities[i] = false;
				continue;
			}

			l_state.unlockedAbilities[i] = (unlockedAbilityFlags[i] != 0);
		}
		if (hadInvalidUnlockFlag)
			std::cerr << "SaveManager: Invalid ability unlock data, coercing to locked." << std::endl;

		AbilityId loadedEquipped = static_cast<AbilityId>(equippedAbility);
		if (!IsValidAbilityId(loadedEquipped, true))
			loadedEquipped = AbilityId::None;

		l_state.equippedAbility = ResolveEquippedAbilityFromUnlocks(l_state.unlockedAbilities, loadedEquipped);
	}
	else
	{
		// Pre-v4 saves have no ability block; infer unlocks from level progression
		// and keep defaults for equipped unless invalid or not unlocked.
		for (const LevelConfig& cfg : GetAllLevels())
		{
			if (cfg.abilityReward == AbilityId::None)
				continue;
			if (l_state.highestUnlockedLevel > cfg.id)
				l_state.unlockedAbilities[GetAbilityIndex(cfg.abilityReward)] = true;
		}

		l_state.equippedAbility = ResolveEquippedAbilityFromUnlocks(
			l_state.unlockedAbilities, l_state.equippedAbility);
	}

	file.close();
}
