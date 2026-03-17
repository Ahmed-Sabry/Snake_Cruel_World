#pragma once

#include "GameState.h"
#include "LevelConfig.h"
#include "Window.h"
#include <unordered_map>
#include <memory>
#include <vector>

class StateManager
{
public:
	StateManager(Window& l_window);
	~StateManager();

	void Update(float l_dt);
	void HandleInput();
	void Render();

	void SwitchTo(StateType l_type);
	void PushState(StateType l_type); // overlay (e.g. pause)
	void PopState();

	// Process deferred state transitions (called between frames)
	void ProcessPendingTransitions();

	Window& GetWindow();

	template <typename T>
	void RegisterState(StateType l_type)
	{
		m_stateFactory[l_type] = [this]() -> std::unique_ptr<BaseState>
		{
			return std::make_unique<T>(*this);
		};
	}

	// Shared data between states
	int currentLevel = 1;
	int score = 0;
	int applesEaten = 0;
	int combo = 0;
	float comboMultiplier = 1.0f;
	int selfCollisions = 0;
	int totalDeaths = 0;
	float levelTime = 0.0f;
	bool levelComplete = false;

	// Persistent progress
	int highestUnlockedLevel = 1;
	int highScores[NUM_LEVELS] = {};
	int starRatings[NUM_LEVELS] = {};

private:
	std::unique_ptr<BaseState> CreateState(StateType l_type);
	void ExecuteSwitchTo(StateType l_type);
	void ExecutePushState(StateType l_type);
	void ExecutePopState();

	Window& m_window;
	std::vector<std::pair<StateType, std::unique_ptr<BaseState>>> m_stateStack;
	std::unordered_map<StateType, std::function<std::unique_ptr<BaseState>()>> m_stateFactory;

	// Deferred transition queue
	enum class TransitionType { Switch, Push, Pop };
	struct PendingTransition
	{
		TransitionType action;
		StateType targetState;
	};
	std::vector<PendingTransition> m_pendingTransitions;
	bool m_isUpdating = false;
};
