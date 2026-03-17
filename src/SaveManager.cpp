#include "SaveManager.h"
#include "StateManager.h"
#include <iostream>

const std::string SaveManager::s_saveFile = "save.dat";

void SaveManager::Save(const StateManager& l_state)
{
	std::ofstream file(s_saveFile, std::ios::binary);
	if (!file.is_open())
		return;

	// Version marker
	int version = 1;
	file.write(reinterpret_cast<const char*>(&version), sizeof(version));

	// Progress
	file.write(reinterpret_cast<const char*>(&l_state.highestUnlockedLevel), sizeof(l_state.highestUnlockedLevel));
	file.write(reinterpret_cast<const char*>(l_state.highScores), sizeof(l_state.highScores));
	file.write(reinterpret_cast<const char*>(l_state.starRatings), sizeof(l_state.starRatings));
	file.write(reinterpret_cast<const char*>(&l_state.totalDeaths), sizeof(l_state.totalDeaths));

	file.close();
}

void SaveManager::Load(StateManager& l_state)
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
	}

	// Validate loaded data
	if (l_state.highestUnlockedLevel < 1 || l_state.highestUnlockedLevel > NUM_LEVELS)
		l_state.highestUnlockedLevel = 1;

	file.close();
}
