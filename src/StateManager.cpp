#include "StateManager.h"

#include <algorithm>
#include <limits>

namespace
{
	constexpr int kMaxStarsPerLevel = 3;

	StateManager::LevelProgress SanitizeLevelProgressFromSave(int l_levelId,
															  const StateManager::LevelProgress& l_src)
	{
		StateManager::LevelProgress out = l_src;
		out.bestScore = std::clamp(l_src.bestScore, 0, std::numeric_limits<int>::max());
		out.bestStars = std::clamp(l_src.bestStars, 0, kMaxStarsPerLevel);
		if (l_levelId == 1)
			out.bossDefeated = false;
		if (l_levelId < 2 || l_levelId > 9)
			out.pageHealed = false;
		if (out.pageHealed)
			out.bossDefeated = true;
		const bool hasProgress =
			(out.bestScore > 0 || out.bestStars > 0 || out.pageHealed ||
			 out.bossDefeated || l_src.stageCompleted);
		out.stageCompleted = hasProgress;
		return out;
	}

	void UpdateLevelPerformance(StateManager::LevelProgress& l_progress, int l_score, int l_stars)
	{
		l_progress.bestScore = std::max(l_progress.bestScore, l_score);
		l_progress.bestStars = std::max(
			l_progress.bestStars, std::clamp(l_stars, 0, kMaxStarsPerLevel));
	}

	void SyncLegacyMirrorsForLevel(StateManager& l_state, std::size_t l_idx)
	{
		const StateManager::LevelProgress& progress =
			l_state.GetLevelProgress(static_cast<int>(l_idx) + 1);
		l_state.highScores[l_idx] = std::max(l_state.highScores[l_idx], progress.bestScore);
		l_state.starRatings[l_idx] = std::max(l_state.starRatings[l_idx], progress.bestStars);
	}
}

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
	campaignProgress[static_cast<std::size_t>(l_levelId - 1)] =
		SanitizeLevelProgressFromSave(l_levelId, l_progress);
}

void StateManager::ClearCampaignProgressEntries()
{
	for (LevelProgress& p : campaignProgress)
		p = LevelProgress{};
}

bool StateManager::HasCompletedLevel(int l_levelId) const
{
	return GetLevelProgress(l_levelId).stageCompleted;
}

bool StateManager::HasDefeatedBoss(int l_levelId) const
{
	if (l_levelId < 2 || l_levelId > NUM_LEVELS)
		return false;
	return GetLevelProgress(l_levelId).bossDefeated;
}

bool StateManager::IsPageHealed(int l_levelId) const
{
	if (l_levelId < 2 || l_levelId > 9)
		return false;
	const LevelProgress& progress = GetLevelProgress(l_levelId);
	return progress.pageHealed || progress.bossDefeated;
}

bool StateManager::HasUnlockedStageSelect() const
{
	if (GetLevelProgress(1).stageCompleted || highestUnlockedLevel > 1)
		return true;

	for (int levelId = 2; levelId <= 10; ++levelId)
	{
		const LevelProgress& progress = GetLevelProgress(levelId);
		if (progress.stageCompleted || progress.bossDefeated || progress.pageHealed)
			return true;
	}
	return false;
}

bool StateManager::CanAccessCampaignLevel(int l_levelId) const
{
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return false;
	if (l_levelId == 1)
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
		const LevelProgress& progress = GetLevelProgress(levelId);
		if (progress.bossDefeated || progress.pageHealed)
			++healedPages;
	}
	return healedPages;
}

int StateManager::GetCompletedLevelCount() const
{
	int completedCount = 0;
	for (std::size_t i = 0; i < campaignProgress.size(); ++i)
	{
		const int levelId = static_cast<int>(i) + 1;
		const LevelProgress& progress = campaignProgress[i];
		const bool fullyCompleted = (levelId >= 2 && levelId <= 9)
			? (progress.bossDefeated || progress.pageHealed)
			: progress.stageCompleted;
		if (fullyCompleted)
			++completedCount;
	}
	return completedCount;
}

bool StateManager::IsL10Unlocked() const
{
	// Pre-v5 saves used highestUnlockedLevel == NUM_LEVELS when the finale was
	// reachable on the linear ladder; preserve that without requiring 8 healed
	// pages or a recorded L10 clear.
	return GetHealedPageCount() >= 8 || HasCompletedLevel(10) ||
		highestUnlockedLevel >= NUM_LEVELS;
}

void StateManager::RecordStageCompletion(int l_levelId, int l_score, int l_stars)
{
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return;

	const std::size_t idx = static_cast<std::size_t>(l_levelId - 1);
	LevelProgress& progress = campaignProgress[idx];
	progress.stageCompleted = true;
	UpdateLevelPerformance(progress, l_score, l_stars);
	SyncLegacyMirrorsForLevel(*this, idx);
	SyncLegacyProgress();
}

void StateManager::RecordBossDefeat(int l_levelId, int l_score, int l_stars, bool l_healPage)
{
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return;

	const std::size_t idx = static_cast<std::size_t>(l_levelId - 1);
	LevelProgress& progress = campaignProgress[idx];
	progress.stageCompleted = true;
	if (l_levelId >= 2)
		progress.bossDefeated = true;
	if (l_healPage && l_levelId >= 2 && l_levelId <= 9)
		progress.pageHealed = true;
	UpdateLevelPerformance(progress, l_score, l_stars);
	SyncLegacyMirrorsForLevel(*this, idx);
	SyncLegacyProgress();
}

void StateManager::RecordLevelCompletion(int l_levelId, int l_score, int l_stars, bool l_healPage)
{
	if (l_levelId < 1 || l_levelId > NUM_LEVELS)
		return;

	const std::size_t idx = static_cast<std::size_t>(l_levelId - 1);
	LevelProgress& progress = campaignProgress[idx];
	progress.stageCompleted = true;
	UpdateLevelPerformance(progress, l_score, l_stars);

	if (l_healPage && l_levelId >= 2 && l_levelId <= 9)
	{
		progress.bossDefeated = true;
		progress.pageHealed = true;
	}

	SyncLegacyMirrorsForLevel(*this, idx);
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
		if (progress.bossDefeated)
			progress.stageCompleted = true;
		if (levelId >= 2 && levelId <= 9 && progress.pageHealed)
		{
			progress.bossDefeated = true;
			progress.stageCompleted = true;
		}

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
	// Derive finale access from loaded campaignProgress only — do not consult
	// highestUnlockedLevel here; it still holds the pre-export v1 mirror and can
	// be stale relative to V5 page/heal state.
	const bool finaleUnlockedFromCampaign =
		GetHealedPageCount() >= 8 || HasCompletedLevel(10);
	if (finaleUnlockedFromCampaign)
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
