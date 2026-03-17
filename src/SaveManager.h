#pragma once

#include <string>
#include <fstream>

class StateManager; // forward declaration

class SaveManager
{
public:
	static void Save(const StateManager& l_state);
	static void Load(StateManager& l_state);

private:
	static const std::string s_saveFile;
};
