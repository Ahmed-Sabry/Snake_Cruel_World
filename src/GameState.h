#pragma once

#include "Window.h"
#include <memory>
#include <functional>

enum class StateType
{
	Splash,
	MainMenu,
	StageSelect,
	LevelSelect,
	Gameplay,
	Pause,
	GameOver,
	Settings,
	Achievements,
	Statistics,
	SkinSelect,
	Cutscene,
	CutsceneGallery
};

class StateManager; // forward declaration

class BaseState
{
public:
	BaseState(StateManager& l_stateManager) : m_stateManager(l_stateManager) {}
	virtual ~BaseState() = default;

	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
	virtual void HandleInput() = 0;
	virtual void Update(float l_dt) = 0;
	virtual void Render() = 0;

protected:
	StateManager& m_stateManager;
};
