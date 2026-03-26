#pragma once

#include "Ability.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <string>

class Snake;
class World;

enum class BossLifecycleState
{
	Dormant,
	TransitionIn,
	Intro,
	Active,
	PhaseTransition, // reserved for scripted multi-phase transitions (not used by base Boss yet)
	Defeated,
	Resolved
};

enum class BossProgressPresentationType
{
	HitPoints,
	Cracks,
	Seals,
	Tentacles,
	PurgeSteps,
	Anchors,
	Custom
};

enum class BossEncounterTrigger
{
	None,
	StageClear
};

enum class BossHazardIntensityMode
{
	Unchanged,
	BossEscalation,
	Disabled
};

enum class BossInteractionType
{
	Reveal,
	FreezeTimers,
	Retarget,
	Stun,
	CounterHit,
	Purify,
	Custom
};

enum class BossProgressEventType
{
	None,
	StageCleared,
	AppleCollected,
	AbilityActivated,
	Custom
};

struct BossArenaBounds
{
	int topInsetTiles = 0;
	int rightInsetTiles = 0;
	int bottomInsetTiles = 0;
	int leftInsetTiles = 0;
};

struct BossArenaRequirements
{
	bool usesBossArena = false;
	BossArenaBounds bossArenaBounds{};
	bool disableStageShrink = false;
	bool allowBossSpecificSpawns = false; // reserved for boss-tuned apple/hazard spawns (see World)
	BossHazardIntensityMode hazardIntensityMode = BossHazardIntensityMode::Unchanged;
};

struct BossConfig
{
	bool enabled = false;
	std::string bossId;
	std::string displayName;
	BossEncounterTrigger trigger = BossEncounterTrigger::None;
	BossProgressPresentationType progressPresentationType = BossProgressPresentationType::HitPoints;
	int progressMax = 0;
	AbilityId counterAbility = AbilityId::None;
	int strongCounterProgress = 2;
	float transitionInDurationSec = 0.35f;
	float introDurationSec = 0.75f;
	float defeatDurationSec = 0.75f;
	BossArenaRequirements arena{};
	bool healPageOnDefeat = true;
	std::string rewardCutsceneId;
};

struct BossRewardHandoff
{
	bool healPage = false;
	std::string cutsceneId;
};

struct BossContext
{
	int levelId = 0;
	int applesEaten = 0;
	float encounterTimeSec = 0.0f;
	float blockSize = 0.0f;
	AbilityId activeAbility = AbilityId::None;
	const World* world = nullptr;
	const Snake* snake = nullptr;
};

struct BossProgressEvent
{
	BossProgressEventType type = BossProgressEventType::None;
	BossInteractionType interactionType = BossInteractionType::Custom;
	AbilityId ability = AbilityId::None;
	int amount = 1;
	int gridX = 0;
	int gridY = 0;
};

struct BossProgressResult
{
	bool progressApplied = false;
	bool phaseAdvanced = false;
	bool defeated = false;
	int progressCurrent = 0;
	int phaseIndex = 0;
};

class Boss
{
public:
	explicit Boss(const BossConfig& l_config);
	virtual ~Boss() = default;

	virtual bool CanStartEncounter(const BossContext& l_ctx) const;
	virtual void BeginEncounter(const BossContext& l_ctx);
	virtual void Update(float l_dt, const BossContext& l_ctx);
	virtual void RenderTo(sf::RenderTarget& l_target, float l_gameTime, const BossContext& l_ctx) const;

	virtual bool CanAcceptProgressEvent(const BossProgressEvent& l_event, const BossContext& l_ctx) const;
	BossProgressResult ApplyProgress(const BossProgressEvent& l_event, const BossContext& l_ctx);

	virtual bool CanBeDamagedByAbility(AbilityId l_ability, const BossContext& l_ctx) const;
	virtual void OnAbilityInteraction(AbilityId l_ability, BossInteractionType l_type, const BossContext& l_ctx);
	virtual void OnDefeated(const BossContext& l_ctx);
	virtual BossRewardHandoff BuildRewardHandoff(const BossContext& l_ctx) const;

	const BossConfig& GetConfig() const { return m_config; }
	const std::string& GetId() const { return m_config.bossId; }
	const std::string& GetDisplayName() const { return m_config.displayName; }
	BossLifecycleState GetLifecycleState() const { return m_state; }
	BossProgressPresentationType GetProgressPresentationType() const { return m_config.progressPresentationType; }
	const BossArenaRequirements& GetArenaRequirements() const { return m_config.arena; }
	int GetCurrentPhaseIndex() const { return m_phaseIndex; }
	int GetProgressCurrent() const { return m_progressCurrent; }
	int GetProgressMax() const { return m_progressMax; }
	float GetStateElapsedSec() const { return m_stateElapsedSec; }
	bool IsResolved() const { return m_state == BossLifecycleState::Resolved; }
	bool IsDefeated() const
	{
		return m_state == BossLifecycleState::Defeated ||
			m_state == BossLifecycleState::Resolved;
	}

protected:
	void SetLifecycleState(BossLifecycleState l_state);
	void SetProgressMax(int l_progressMax);
	void SetInvulnerable(bool l_invulnerable);
	void MarkResolved();

	// Counter-ability damage should respect CanBeDamagedByAbility (base implementation does).
	virtual int EvaluateProgressDelta(const BossProgressEvent& l_event, const BossContext& l_ctx) const;
	virtual bool ShouldAdvancePhase(const BossContext& l_ctx) const;
	virtual void AdvancePhase(const BossContext& l_ctx);

private:
	BossConfig m_config;
	BossLifecycleState m_state;
	int m_phaseIndex;
	int m_progressCurrent;
	int m_progressMax;
	bool m_invulnerable;
	float m_stateElapsedSec;
};
