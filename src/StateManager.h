#pragma once

#include "GameState.h"
#include "LevelConfig.h"
#include "Window.h"
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

class AudioManager;
class StatsManager;
class AchievementManager;

class StateManager
{
public:
	StateManager(Window& l_window, AudioManager& l_audio,
				 StatsManager& l_stats, AchievementManager& l_achievements);
	~StateManager();

	void Update(float l_dt);
	void HandleInput();
	void Render();

	void SwitchTo(StateType l_type);
	void PushState(StateType l_type); // overlay (e.g. pause)
	void PopState();

	Window& GetWindow();
	AudioManager& GetAudio();
	StatsManager& GetStats();
	AchievementManager& GetAchievements();

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

	// Gamification state
	int activeSkinIndex = 0;
	bool endlessMode = false;

	// Cutscene routing (set before SwitchTo(Cutscene))
	std::string cutsceneId;
	StateType cutsceneReturnState = StateType::MainMenu;
	std::function<void(StateManager&)> cutsceneOnSkip; // called when player skips with Escape
	bool introPlayed = false;

	// Death context (set by PlayState::OnDeath, read by GameOverState)
	enum class DeathCause { Wall, Predator, MirrorGhost, Unknown };
	struct DeathContext
	{
		DeathCause cause = DeathCause::Unknown;
		int appleCount = 0;             // apples eaten before death
		int retryCount = 0;             // consecutive retries of same level
		int sessionBestApples = 0;      // best apple count in current retry streak
		bool sessionBestImproved = false; // strict improvement over previous best
		bool wasInBlackout = false;     // died during blackout
		bool wasOnQuicksand = false;    // died while on quicksand
		bool hadHighCombo = false;      // had 2x+ combo when died
		int comboLostAt = 0;            // consecutive apple count when died
		int lastPlayedLevel = -1;       // tracks level for retry detection
	};
	DeathContext deathCtx;

private:
	void ProcessPendingTransitions();
	std::unique_ptr<BaseState> CreateState(StateType l_type);
	void ExecuteSwitchTo(StateType l_type);
	void ExecutePushState(StateType l_type);
	void ExecutePopState();

	Window& m_window;
	AudioManager& m_audio;
	StatsManager& m_stats;
	AchievementManager& m_achievements;
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
