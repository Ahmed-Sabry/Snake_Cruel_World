#include "StateManager.h"

#include <algorithm>

StateManager::StateManager(Window& l_window, AudioManager& l_audio,
						   StatsManager& l_stats, AchievementManager& l_achievements)
	: m_window(l_window), m_audio(l_audio),
	  m_stats(l_stats), m_achievements(l_achievements)
{
}

StateManager::~StateManager()
{
}

void StateManager::Update(float l_dt)
{
	if (m_stateStack.empty())
		return;

	m_isUpdating = true;
	m_stateStack.back().second->Update(l_dt);
	m_isUpdating = false;

	ProcessPendingTransitions();
}

void StateManager::HandleInput()
{
	if (m_stateStack.empty())
		return;

	m_isUpdating = true;
	m_stateStack.back().second->HandleInput();
	m_isUpdating = false;

	ProcessPendingTransitions();
}

void StateManager::Render()
{
	if (m_stateStack.empty())
		return;

	m_window.Clear();

	m_isUpdating = true;
	// Render all states in the stack (bottom to top) so overlays work
	for (auto& pair : m_stateStack)
	{
		pair.second->Render();
	}
	m_isUpdating = false;

	ProcessPendingTransitions();

	m_window.Display();
}

void StateManager::SwitchTo(StateType l_type)
{
	if (m_isUpdating)
	{
		// Defer: the current state is still executing, can't destroy it yet
		m_pendingTransitions.push_back({ TransitionType::Switch, l_type });
		return;
	}
	ExecuteSwitchTo(l_type);
}

void StateManager::PushState(StateType l_type)
{
	if (m_isUpdating)
	{
		m_pendingTransitions.push_back({ TransitionType::Push, l_type });
		return;
	}
	ExecutePushState(l_type);
}

void StateManager::PopState()
{
	if (m_isUpdating)
	{
		m_pendingTransitions.push_back({ TransitionType::Pop, StateType::MainMenu /*unused*/ });
		return;
	}
	ExecutePopState();
}

const StateManager::LevelProgress& StateManager::GetLevelProgress(int l_levelId) const
{
	static const LevelProgress s_emptyProgress{};
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return s_emptyProgress;
	return campaignProgress[static_cast<std::size_t>(l_levelId - 1)];
}

void StateManager::SetLevelProgressFromSave(int l_levelId, const LevelProgress& l_progress)
{
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return;
	campaignProgress[static_cast<std::size_t>(l_levelId - 1)] = l_progress;
}

void StateManager::ResetAllCampaignProgress()
{
	for (LevelProgress& p : campaignProgress)
		p = LevelProgress{};
}

bool StateManager::HasCompletedLevel(int l_levelId) const
{
	return GetLevelProgress(l_levelId).stageCompleted;
}

bool StateManager::IsPageHealed(int l_levelId) const
{
	if (l_levelId < 2 || l_levelId > 9)
		return false;
	return GetLevelProgress(l_levelId).pageHealed;
}

bool StateManager::HasUnlockedStageSelect() const
{
	if (GetLevelProgress(1).stageCompleted || highestUnlockedLevel > 1)
		return true;

	for (int levelId = 2; levelId <= 10; ++levelId)
	{
		const LevelProgress& progress = GetLevelProgress(levelId);
		if (progress.stageCompleted || progress.pageHealed)
			return true;
	}
	return false;
}

bool StateManager::CanAccessCampaignLevel(int l_levelId) const
{
	if (l_levelId <= 1)
		return true;
	if (l_levelId >= 2 && l_levelId <= 9)
		return HasUnlockedStageSelect();
	if (l_levelId == 10)
		return IsL10Unlocked();
	return false;
}

int StateManager::GetHealedPageCount() const
{
	int healedPages = 0;
	for (int levelId = 2; levelId <= 9; ++levelId)
	{
		if (GetLevelProgress(levelId).pageHealed)
			++healedPages;
	}
	return healedPages;
}

int StateManager::GetCompletedLevelCount() const
{
	int completedCount = 0;
	for (const LevelProgress& progress : campaignProgress)
	{
		if (progress.stageCompleted)
			++completedCount;
	}
	return completedCount;
}

bool StateManager::IsL10Unlocked() const
{
	return GetHealedPageCount() >= 8 || HasCompletedLevel(10);
}

void StateManager::RecordLevelCompletion(int l_levelId, int l_score, int l_stars, bool l_healPage)
{
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return;

	const std::size_t idx = static_cast<std::size_t>(l_levelId - 1);
	LevelProgress& progress = campaignProgress[idx];
	progress.stageCompleted = true;
	progress.bestScore = std::max(progress.bestScore, l_score);
	progress.bestStars = std::max(progress.bestStars, std::clamp(l_stars, 0, 3));

	highScores[idx] = std::max(highScores[idx], progress.bestScore);
	starRatings[idx] = std::max(starRatings[idx], progress.bestStars);

	if (l_healPage && l_levelId >= 2 && l_levelId <= 9)
		progress.pageHealed = true;

	SyncLegacyProgress();
}

void StateManager::SyncLegacyProgress()
{
	int legacyHighest = 1;

	for (std::size_t i = 0; i < campaignProgress.size(); ++i)
	{
		LevelProgress& progress = campaignProgress[i];
		const int levelId = static_cast<int>(i) + 1;

		progress.bestScore = std::max(progress.bestScore, highScores[i]);
		progress.bestStars = std::clamp(std::max(progress.bestStars, starRatings[i]), 0, 3);

		highScores[i] = std::max(highScores[i], progress.bestScore);
		starRatings[i] = std::max(starRatings[i], progress.bestStars);

		if (progress.bestScore > 0 || progress.bestStars > 0)
			progress.stageCompleted = true;
		if (levelId == 1 && highestUnlockedLevel > 1)
			progress.stageCompleted = true;
		if (levelId >= 2 && levelId <= 9 && progress.pageHealed)
			progress.stageCompleted = true;

		if (progress.stageCompleted)
			legacyHighest = std::max(legacyHighest, levelId);
	}

	if (GetLevelProgress(1).stageCompleted)
		legacyHighest = std::max(legacyHighest, 2);
	if (IsL10Unlocked())
		legacyHighest = std::max(legacyHighest, NUM_LEVELS);

	highestUnlockedLevel = std::clamp(std::max(highestUnlockedLevel, legacyHighest), 1, NUM_LEVELS);
}

void StateManager::ExportCampaignProgressToLegacy()
{
	int legacyHighest = 1;
	for (std::size_t i = 0; i < campaignProgress.size(); ++i)
	{
		const LevelProgress& progress = campaignProgress[i];
		const int levelId = static_cast<int>(i) + 1;
		highScores[i] = progress.bestScore;
		starRatings[i] = std::clamp(progress.bestStars, 0, 3);
		if (progress.stageCompleted)
			legacyHighest = std::max(legacyHighest, levelId);
	}
	if (GetLevelProgress(1).stageCompleted)
		legacyHighest = std::max(legacyHighest, 2);
	if (IsL10Unlocked())
		legacyHighest = std::max(legacyHighest, NUM_LEVELS);
	highestUnlockedLevel = std::clamp(legacyHighest, 1, NUM_LEVELS);
}

void StateManager::ProcessPendingTransitions()
{
	while (!m_pendingTransitions.empty())
	{
		auto transition = m_pendingTransitions.front();
		m_pendingTransitions.erase(m_pendingTransitions.begin());

		switch (transition.action)
		{
			case TransitionType::Switch:
				ExecuteSwitchTo(transition.targetState);
				break;
			case TransitionType::Push:
				ExecutePushState(transition.targetState);
				break;
			case TransitionType::Pop:
				ExecutePopState();
				break;
			default:
				break;
		}
	}
}

Window& StateManager::GetWindow()
{
	return m_window;
}

AudioManager& StateManager::GetAudio()
{
	return m_audio;
}

StatsManager& StateManager::GetStats()
{
	return m_stats;
}

AchievementManager& StateManager::GetAchievements()
{
	return m_achievements;
}

void StateManager::ExecuteSwitchTo(StateType l_type)
{
	// Create the new state first to avoid clearing the stack on failure
	auto state = CreateState(l_type);
	if (!state)
		return;

	m_isUpdating = true;
	while (!m_stateStack.empty())
	{
		m_stateStack.back().second->OnExit();
		m_stateStack.pop_back();
	}
	m_isUpdating = false;
	ProcessPendingTransitions();

	m_stateStack.push_back({ l_type, std::move(state) });
	m_isUpdating = true;
	m_stateStack.back().second->OnEnter();
	m_isUpdating = false;
	ProcessPendingTransitions();
}

void StateManager::ExecutePushState(StateType l_type)
{
	auto state = CreateState(l_type);
	if (state)
	{
		m_stateStack.push_back({ l_type, std::move(state) });
		m_isUpdating = true;
		m_stateStack.back().second->OnEnter();
		m_isUpdating = false;
		ProcessPendingTransitions();
	}
}

void StateManager::ExecutePopState()
{
	if (m_stateStack.empty())
		return;
	m_isUpdating = true;
	m_stateStack.back().second->OnExit();
	m_isUpdating = false;
	m_stateStack.pop_back();
	ProcessPendingTransitions();
}

std::unique_ptr<BaseState> StateManager::CreateState(StateType l_type)
{
	auto it = m_stateFactory.find(l_type);
	if (it != m_stateFactory.end())
	{
		return it->second();
	}
	return nullptr;
}
