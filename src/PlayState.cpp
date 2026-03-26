#include "PlayState.h"
#include "AudioManager.h"
#include "InkRenderer.h"
#include "StatsManager.h"
#include "AchievementManager.h"
#include "SnakeSkin.h"
#include "bosses/PlaceholderBoss.h"
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace {
	constexpr float kAppleBurstDuration = 0.2f;

	std::unique_ptr<Boss> CreateBoss(const BossConfig& l_config)
	{
		// Data-driven hook: map bossId to concrete types as they are added.
		(void)l_config.bossId;
		return std::make_unique<PlaceholderBoss>(l_config);
	}
}

PlayState::PlayState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_world(l_stateManager.GetWindow(), m_snake),
	  m_hud(l_stateManager.GetWindow().GetWindowSize()),
	  m_abilityHud(l_stateManager.GetWindow().GetWindowSize()),
	  m_predatorApplesEaten(0),
	  m_psychedelicTimer(0.0f),
	  m_elapsedTime(0.0f),
	  m_gameTime(0.0f),
	  m_applesEaten(0),
	  m_consecutiveApples(0),
	  m_speedModifier(1.0f),
	  m_lastShrinkCount(0),
	  m_cheatExtend(false),
	  m_escReleased(true),
	  m_rReleased(true),
	  m_cycleReleased(true),
	  m_activateReleased(true),
	  m_comboSoundPlayed(false),
	  m_levelCompleteDelay(-1.0f),
	  m_encounterPhase(EncounterPhase::Stage),
	  m_activeBoss(nullptr),
	  m_cruelPhase(0),
	  m_screenFlipped(false),
	  m_phaseAnnouncementTimer(0.0f),
	  m_announcementDuration(2.0f),
	  m_announcementCharSize(48),
	  m_announcementFontLoaded(false),
	  m_postProcessorInited(false),
	  m_pageTurnTimer(0.0f),
	  m_pageTurnDuration(0.5f),
	  m_deathInkRunTimer(0.0f),
	  m_deathInkRunDuration(0.3f),
	  m_deathInkRunActive(false),
	  m_appleBurstTimer(0.0f),
	  m_borderHatchTimer(0.0f),
	  m_borderHatchDuration(0.3f),
	  m_quicksandTouches(0),
	  m_wasOnQuicksand(false),
	  m_timedAppleMisses(0),
	  m_poisonApplesThisLevel(0),
	  m_reachedMinBody(false),
	  m_screenFlipStartTime(0.0f),
	  m_heartbeatTimer(0.0f),
	  m_wasInGracePeriod(false),
	  m_endlessWarningTimer(0.0f)
{
}

void PlayState::OnEnter()
{
	Window& window = m_stateManager.GetWindow();

	// Load level config
	auto levels = GetAllLevels();
	int idx = m_stateManager.currentLevel - 1;
	if (idx < 0 || idx >= NUM_LEVELS) idx = 0;
	m_stateManager.currentLevel = idx + 1;
	m_levelConfig = levels[idx];

	// Apply level palette — use paper tone as window clear color for ink style fallback
	window.SetBackground(m_levelConfig.paperTone);

	// Track retries for context-sensitive taunts
	{
		auto& dc = m_stateManager.deathCtx;
		if (m_stateManager.currentLevel == dc.lastPlayedLevel)
			dc.retryCount++;
		else
		{
			dc.retryCount = 0;
			dc.sessionBestApples = 0;
		}
		dc.lastPlayedLevel = m_stateManager.currentLevel;
	}
	m_stateManager.deathCtx.cause = StateManager::DeathCause::Unknown;

	// Reset game state
	m_snake.Reset();
	m_world.SetTopOffset(HUD::GetHeight());
	m_world.Reset(window, m_snake);

	// Endless mode overrides must precede world init so shrink params,
	// level ID, and first apple spawn use the correct endless config
	if (m_stateManager.endlessMode)
	{
		m_levelConfig.applesToWin = 99999; // effectively infinite
		m_levelConfig.shrinkTimerSec = 15.0f;
		m_levelConfig.shrinkInterval = 0; // timer-based only
		m_levelConfig.baseSpeed = 10.0f;
		// Clear all mechanic flags — EndlessModeController toggles them dynamically
		m_levelConfig.hasBlackouts = false;
		m_levelConfig.hasQuicksand = false;
		m_levelConfig.hasMirrorGhost = false;
		m_levelConfig.hasTimedApples = false;
		m_levelConfig.hasPoisonApples = false;
		m_levelConfig.hasEarthquakes = false;
		m_levelConfig.hasPredator = false;
		m_levelConfig.hasControlShuffle = false;
	}

	// Set shrink parameters before first RespawnApple so the pre-shrink
	// safety margin uses the configured interval, not the default
	m_world.SetShrinkInterval(m_levelConfig.shrinkInterval);
	m_world.SetShrinkTimerSec(m_levelConfig.shrinkTimerSec);

	m_world.SetLevelId(m_levelConfig.id);
	m_world.SetAppleColor(m_levelConfig.apple);
	m_world.RespawnApple(m_snake);
	m_world.SetBorderColor(m_levelConfig.border);
	m_hud.SetLevelColors(m_levelConfig.paperTone, m_levelConfig.inkTint, m_levelConfig.accentColor);
	m_abilityHud.SetLevelColors(m_levelConfig.paperTone, m_levelConfig.inkTint, m_levelConfig.accentColor);
	m_snake.SetColors(m_levelConfig.snakeHead, m_levelConfig.snakeBody);

	// Apply active skin based on three-tier rule:
	// - First playthrough (level not yet beaten): level colors only
	// - Replayed levels (already beaten): skin applies
	// - Endless mode: skin always applies
	{
		int skinIdx = m_stateManager.activeSkinIndex;
		bool shouldApplySkin = false;

		if (skinIdx > 0)
		{
			if (m_stateManager.endlessMode)
				shouldApplySkin = true;
			else
				shouldApplySkin = HasBossEncounter()
					? m_stateManager.HasDefeatedBoss(m_stateManager.currentLevel)
					: m_stateManager.HasCompletedLevel(m_stateManager.currentLevel);
		}

		auto skins = GetAllSkins();
		if (shouldApplySkin && skinIdx < (int)skins.size())
			m_snake.ApplySkin(skins[skinIdx]);
		else
			m_snake.ClearSkin();
	}

	m_elapsedTime = 0.0f;
	m_gameTime = 0.0f;
	m_applesEaten = 0;
	m_consecutiveApples = 0;
	m_speedModifier = 1.0f;
	m_lastShrinkCount = 0;
	m_cheatExtend = false;
	m_escReleased = true;
	m_rReleased = true;
	m_cycleReleased = true;
	m_activateReleased = true;
	m_comboSoundPlayed = false;
	m_levelCompleteDelay = -1.0f;
	m_encounterPhase = EncounterPhase::Stage;
	m_activeBoss.reset();
	m_bossEncounterStartTime = 0.0f;
	m_abilityController.LoadPersistentProgress(
		m_stateManager.unlockedAbilities, m_stateManager.equippedAbility);
	SyncAbilityState();

	m_stateManager.score = 0;
	m_stateManager.applesEaten = 0;
	m_stateManager.combo = 0;
	m_stateManager.comboMultiplier = 1.0f;
	m_stateManager.selfCollisions = 0;
	m_stateManager.levelTime = 0.0f;
	m_stateManager.levelComplete = false;

	m_mirrorFlipCounter = 0;

	// Achievement/statistics tracking resets
	m_quicksandTouches = 0;
	m_wasOnQuicksand = false;
	m_timedAppleMisses = 0;
	m_poisonApplesThisLevel = 0;
	m_reachedMinBody = false;
	m_screenFlipStartTime = 0.0f;

	// Notify stats of level start (skip for endless — tracked separately)
	if (!m_stateManager.endlessMode)
		m_stateManager.GetStats().OnLevelStart(m_stateManager.currentLevel);

	// Load font for notifications and announcements (once)
	if (!m_announcementFontLoaded)
	{
		if (m_announcementFont.loadFromFile(FONT_PATH))
			m_announcementFontLoaded = true;
	}

	// Init achievement notification
	if (m_announcementFontLoaded)
		m_achievementNotif.Init(m_announcementFont);

	// Endless mode controller setup (config overrides already applied above)
	m_endlessWarningTimer = 0.0f;
	m_endlessWarningText.clear();
	if (m_stateManager.endlessMode)
	{
		m_endlessCtrl = std::make_unique<EndlessModeController>(
			m_stateManager);
	}
	else
	{
		m_endlessCtrl.reset();
	}

	m_particles.Clear();
	m_screenShake.Reset(m_stateManager.GetWindow());

	// Initialize level mechanic systems
	if (m_levelConfig.hasBlackouts)
		m_blackout.Reset();

	if (m_levelConfig.hasQuicksand)
	{
		float maxThick = std::max({m_world.GetEffectiveThickness(0), m_world.GetEffectiveThickness(1),
								   m_world.GetEffectiveThickness(2), m_world.GetEffectiveThickness(3)});
		m_quicksand.Reset(m_world.GetMaxX(), m_world.GetMaxY(),
						  maxThick, m_snake.GetBlockSize(),
						  m_world.GetTopOffset());
	}

	if (m_levelConfig.hasMirrorGhost)
		m_mirrorGhost.Reset();

	if (m_levelConfig.hasTimedApples)
		m_timedApple.Reset(m_levelConfig.appleTimerSec);

	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.Reset(m_snake.GetBlockSize());
		m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());
	}

	if (m_levelConfig.hasEarthquakes)
		m_earthquake.Reset(m_snake.GetBlockSize());

	if (m_levelConfig.hasPredator)
	{
		m_predator.Reset(m_snake.GetBlockSize(), m_snake, m_world);
		m_predatorApplesEaten = 0;
	}

	if (m_levelConfig.hasControlShuffle)
	{
		m_controlShuffle.Reset();
		m_controlShuffle.SetColors(m_levelConfig.paperTone, m_levelConfig.inkTint);
	}

	m_psychedelicTimer = 0.0f;

	// Level 10 "Cruel World": override to Phase 1 after all mechanics initialized
	if (m_levelConfig.id == 10)
		InitCruelWorldPhases();

	// --- Initialize "Living Ink" visual systems ---
	m_snake.SetUseInkStyle(true);
	m_snake.SetCorruption(m_levelConfig.corruption);
	m_snake.SetInkTint(m_levelConfig.inkTint);

	m_world.SetUseInkStyle(true);
	m_world.SetCorruption(m_levelConfig.corruption);
	m_world.SetInkTint(m_levelConfig.inkTint);
	m_world.SetAccentColor(m_levelConfig.accentColor);

	// Generate paper background
	sf::Vector2u winSize = window.GetWindowSize();
	m_paperBackground.Generate(m_levelConfig, winSize.x, winSize.y);

	// Initialize post-processor (only once)
	if (!m_postProcessorInited)
	{
		m_postProcessorInited = m_postProcessor.Init(winSize.x, winSize.y);
	}
	m_postProcessor.Configure(m_levelConfig);

	// Start page-turn entry animation
	m_pageTurnTimer = m_pageTurnDuration;
	m_deathInkRunActive = false;
	m_deathInkRunTimer = 0.0f;
	m_appleBurstTimer = 0.0f;
	m_borderHatchTimer = 0.0f;
	m_heartbeatTimer = 0.0f;

	// Clear any stale announcement from previous run before deciding whether to show subtitle
	m_phaseAnnouncementText.clear();
	m_phaseAnnouncementTimer = 0.0f;
	m_announcementDuration = 0.0f;
	m_announcementCharSize = 48;

	// Show level subtitle as entry announcement (first 3 attempts only — gets stale after that)
	if (!m_stateManager.endlessMode && !m_levelConfig.subtitle.empty()
		&& m_stateManager.deathCtx.retryCount < 3)
	{
		m_phaseAnnouncementText = m_levelConfig.subtitle;
		m_announcementDuration = 2.5f;
		m_phaseAnnouncementTimer = m_announcementDuration;
		m_announcementCharSize = 24;
	}
}

void PlayState::InitCruelWorldPhases()
{
	m_cruelPhase = 0;
	m_screenFlipped = false;
	m_phaseAnnouncementTimer = 0.0f;
	m_phaseAnnouncementText.clear();

	// Font is loaded in OnEnter() — no need to load here

	// Phase 1: only timed apples active
	m_levelConfig.hasBlackouts = false;
	m_levelConfig.hasQuicksand = false;
	m_levelConfig.hasPredator = false;
	m_levelConfig.hasEarthquakes = false;
	m_levelConfig.hasControlShuffle = false;
	m_levelConfig.hasPoisonApples = false;
	m_levelConfig.hasTimedApples = true;
	m_levelConfig.hasMirrorGhost = false;

	// Phase 1 parameters
	m_levelConfig.baseSpeed = 12.0f;
	m_levelConfig.shrinkInterval = 4;
	m_levelConfig.shrinkTimerSec = 0.0f;
	m_levelConfig.appleTimerSec = 6.0f;
	m_world.SetShrinkInterval(4);
	m_world.SetShrinkTimerSec(0.0f);

	// Phase 1 theme: L1 "False Hope" callback
	m_levelConfig.background = sf::Color(28, 22, 30);
	m_levelConfig.border = sf::Color(175, 120, 75);
	m_world.SetBorderColor(m_levelConfig.border);

	// Phase 1 ink style: clean notebook, low corruption (not the L10 default of 1.0)
	m_levelConfig.paperTone = sf::Color(248, 242, 228);
	m_levelConfig.inkTint = sf::Color(45, 40, 55);
	m_levelConfig.accentColor = sf::Color(170, 65, 55);
	m_levelConfig.corruption = 0.20f;
	m_hud.SetLevelColors(m_levelConfig.paperTone, m_levelConfig.inkTint, m_levelConfig.accentColor);
	m_abilityHud.SetLevelColors(m_levelConfig.paperTone, m_levelConfig.inkTint, m_levelConfig.accentColor);
	m_stateManager.GetWindow().SetBackground(m_levelConfig.paperTone);

	// Update snake/world with Phase 1 ink params (overrides the L10 defaults set by OnEnter)
	m_snake.SetCorruption(m_levelConfig.corruption);
	m_snake.SetInkTint(m_levelConfig.inkTint);
	m_world.SetCorruption(m_levelConfig.corruption);
	m_world.SetInkTint(m_levelConfig.inkTint);

	// Regenerate paper background for Phase 1 look
	sf::Vector2u winSize = m_stateManager.GetWindow().GetWindowSize();
	m_paperBackground.Generate(m_levelConfig, winSize.x, winSize.y);
	m_postProcessor.Configure(m_levelConfig);
}

void PlayState::AdvanceCruelPhase()
{
	m_cruelPhase++;
	Window& window = m_stateManager.GetWindow();

	switch (m_cruelPhase)
	{
		case 1: // Phase 2 (apples 6-10): Blackouts + Predator
		{
			m_levelConfig.hasBlackouts = true;
			m_levelConfig.hasPredator = true;

			m_blackout.Reset();
			m_predator.Reset(m_snake.GetBlockSize(), m_snake, m_world);
			m_predatorApplesEaten = 0;

			m_levelConfig.appleTimerSec = 5.0f;
			m_levelConfig.baseSpeed = 13.0f;
			m_levelConfig.shrinkInterval = 3;
			m_world.SetShrinkInterval(3);

			// Theme: cold blue-gray (Level 8 callback)
			m_levelConfig.background = sf::Color(14, 16, 26);
			m_levelConfig.border = sf::Color(55, 65, 95);
			m_world.SetBorderColor(m_levelConfig.border);

			m_phaseAnnouncementText = "It gets worse.";
			m_announcementDuration = 2.0f;
			m_phaseAnnouncementTimer = 2.0f;
			m_announcementCharSize = 48;
			break;
		}

		case 2: // Phase 3 (apples 11-15): Quicksand + Poison
		{
			m_levelConfig.hasQuicksand = true;
			m_levelConfig.hasPoisonApples = true;

			float maxThick = std::max({m_world.GetEffectiveThickness(0),
									   m_world.GetEffectiveThickness(1),
									   m_world.GetEffectiveThickness(2),
									   m_world.GetEffectiveThickness(3)});
			m_quicksand.Reset(m_world.GetMaxX(), m_world.GetMaxY(),
							  maxThick, m_snake.GetBlockSize(),
							  m_world.GetTopOffset());

			m_poisonApple.Reset(m_snake.GetBlockSize());
			m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());

			m_levelConfig.appleTimerSec = 4.0f;
			m_levelConfig.baseSpeed = 14.0f;
			m_levelConfig.shrinkInterval = 2;
			m_world.SetShrinkInterval(2);

			// Theme: sickly poisonous green (Level 6 callback)
			m_levelConfig.background = sf::Color(10, 25, 10);
			m_levelConfig.border = sf::Color(40, 110, 30);
			m_world.SetBorderColor(m_levelConfig.border);

			m_phaseAnnouncementText = "It gets worse.";
			m_announcementDuration = 2.0f;
			m_phaseAnnouncementTimer = 2.0f;
			m_announcementCharSize = 48;
			break;
		}

		case 3: // Phase 4 (apples 16-20): Everything. All at once.
		{
			m_levelConfig.hasEarthquakes = true;
			m_levelConfig.hasControlShuffle = true;
			m_levelConfig.hasMirrorGhost = true;

			m_earthquake.Reset(m_snake.GetBlockSize());
			m_controlShuffle.Reset();
			m_controlShuffle.SetColors(m_levelConfig.paperTone, m_levelConfig.inkTint);
			m_mirrorGhost.Reset();
			m_mirrorFlipCounter = 0;

			m_levelConfig.appleTimerSec = 3.0f;
			m_levelConfig.baseSpeed = 15.0f;
			m_levelConfig.shrinkInterval = 2;
			m_levelConfig.shrinkTimerSec = 5.0f;
			m_world.SetShrinkInterval(2);
			m_world.SetShrinkTimerSec(5.0f);

			// Theme: scorched earth -- near-black with crimson borders
			m_levelConfig.background = sf::Color(8, 5, 5);
			m_levelConfig.border = sf::Color(170, 30, 20);
			m_world.SetBorderColor(m_levelConfig.border);

			m_phaseAnnouncementText = "Everything. All at once.";
			m_announcementDuration = 2.0f;
			m_phaseAnnouncementTimer = 2.0f;
			m_announcementCharSize = 48;
			break;
		}

		default:
			break;
	}

	// Common phase transition effects
	m_screenShake.Trigger(0.5f, 5.0f);
	m_stateManager.GetAudio().PlaySound("phase_advance");

	// Update ink-style visuals for the new phase
	// Map L10 phases to escalating corruption and different paper tones
	static const sf::Color phasePaper[] = {
		sf::Color(248, 242, 228), // Phase 1: L1 cream callback
		sf::Color(205, 210, 225), // Phase 2: L8 cold gray callback
		sf::Color(195, 205, 185), // Phase 3: L6 sickly green callback
		sf::Color(195, 170, 155), // Phase 4: scorched parchment
	};
	static const sf::Color phaseInk[] = {
		sf::Color(45, 40, 55),    // Phase 1: blue-black ballpoint
		sf::Color(40, 45, 70),    // Phase 2: slate blue
		sf::Color(25, 55, 20),    // Phase 3: forest green
		sf::Color(85, 25, 15),    // Phase 4: blood red
	};
	static const float phaseCorruption[] = { 0.20f, 0.45f, 0.70f, 1.0f };

	int pi = std::min(m_cruelPhase, 3);
	m_levelConfig.paperTone = phasePaper[pi];
	m_levelConfig.inkTint = phaseInk[pi];
	m_levelConfig.corruption = phaseCorruption[pi];

	// Update HUD and window for new phase
	m_hud.SetLevelColors(m_levelConfig.paperTone, m_levelConfig.inkTint, m_levelConfig.accentColor);
	m_abilityHud.SetLevelColors(m_levelConfig.paperTone, m_levelConfig.inkTint, m_levelConfig.accentColor);
	window.SetBackground(m_levelConfig.paperTone);

	// Regenerate paper background for new phase
	sf::Vector2u winSize = m_stateManager.GetWindow().GetWindowSize();
	m_paperBackground.Generate(m_levelConfig, winSize.x, winSize.y);

	// Update ink params on snake and world
	m_snake.SetCorruption(m_levelConfig.corruption);
	m_snake.SetInkTint(m_levelConfig.inkTint);
	m_world.SetCorruption(m_levelConfig.corruption);
	m_world.SetInkTint(m_levelConfig.inkTint);

	// Update control shuffle indicator colors for new phase
	if (m_levelConfig.hasControlShuffle)
		m_controlShuffle.SetColors(m_levelConfig.paperTone, m_levelConfig.inkTint);

	// Reconfigure post-processor for new corruption level
	m_postProcessor.Configure(m_levelConfig);
}

void PlayState::OnExit()
{
	m_screenShake.Reset(m_stateManager.GetWindow());
}

void PlayState::SyncAbilityState()
{
	const AbilityDefinition* visualDef = m_abilityController.GetVisualDefinition();
	if (visualDef)
		m_snake.SetAbilityVisual(visualDef->visual);
	else
		m_snake.ClearAbilityVisual();

	m_abilityController.ExportPersistentProgress(
		m_stateManager.unlockedAbilities, m_stateManager.equippedAbility);
}

int PlayState::CalculateStars() const
{
	int stars = 1;
	if (m_stateManager.selfCollisions <= m_levelConfig.starThreshold2)
		stars = 2;
	if (m_stateManager.selfCollisions <= m_levelConfig.starThreshold3)
		stars = 3;
	return stars;
}

bool PlayState::HasBossEncounter() const
{
	return !m_stateManager.endlessMode && m_levelConfig.bossConfig.enabled;
}

bool PlayState::StartsBossOnStageClear() const
{
	return m_levelConfig.bossConfig.trigger == BossEncounterTrigger::StageClear;
}

bool PlayState::IsBossRewardQuiesced() const
{
	return m_encounterPhase == EncounterPhase::BossReward;
}

BossContext PlayState::BuildBossContext() const
{
	BossContext ctx{};
	ctx.levelId = m_stateManager.currentLevel;
	ctx.applesEaten = m_applesEaten;
	ctx.encounterTimeSec =
		m_activeBoss ? std::max(0.0f, m_gameTime - m_bossEncounterStartTime) : 0.0f;
	ctx.blockSize = m_snake.GetBlockSize();
	ctx.activeAbility = m_abilityController.GetActive();
	ctx.world = &m_world;
	ctx.snake = &m_snake;
	return ctx;
}

void PlayState::BeginBossEncounter()
{
	if (!HasBossEncounter() || !StartsBossOnStageClear() || m_activeBoss)
		return;

	m_activeBoss = CreateBoss(m_levelConfig.bossConfig);
	m_bossEncounterStartTime = m_gameTime;
	BossContext ctx = BuildBossContext();
	if (!m_activeBoss->CanStartEncounter(ctx))
	{
		m_activeBoss.reset();
		m_bossEncounterStartTime = 0.0f;
		CompleteEncounterVictory(false, "", true);
		return;
	}

	m_stateManager.RecordStagePhaseCleared(
		m_stateManager.currentLevel, m_stateManager.score, CalculateStars());

	m_activeBoss->BeginEncounter(ctx);
	m_encounterPhase = EncounterPhase::BossTransition;
	m_world.SetBossArenaMode(m_activeBoss->GetArenaRequirements(), m_snake.GetBlockSize());
	m_world.Borders(m_stateManager.GetWindow());
	m_world.ClampSnakeToPlayableGrid(m_snake);
	m_snake.ClearSelfCollideFlag();
	if (!m_world.IsAppleInBounds(m_snake.GetBlockSize()))
		m_world.RespawnApple(m_snake);
	if (m_levelConfig.hasPoisonApples && !m_poisonApple.IsInBounds(m_world, m_snake.GetBlockSize()))
		m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());

	m_stateManager.GetAudio().PlaySound("phase_advance");
	m_phaseAnnouncementText = m_levelConfig.bossConfig.displayName;
	m_announcementDuration = 1.2f;
	m_phaseAnnouncementTimer = 1.2f;
	m_announcementCharSize = 28;
}

void PlayState::UpdateBossEncounter(float l_dt)
{
	if (!m_activeBoss)
		return;

	const BossLifecycleState oldState = m_activeBoss->GetLifecycleState();
	m_activeBoss->Update(l_dt, BuildBossContext());
	const BossLifecycleState newState = m_activeBoss->GetLifecycleState();

	if (oldState != newState && newState == BossLifecycleState::Active)
	{
		m_encounterPhase = EncounterPhase::BossCombat;
		m_phaseAnnouncementText = "Boss Active";
		m_announcementDuration = 0.8f;
		m_phaseAnnouncementTimer = 0.8f;
		m_announcementCharSize = 22;
	}

	if (newState == BossLifecycleState::Defeated)
		m_encounterPhase = EncounterPhase::BossReward;

	if (m_activeBoss->IsResolved() && !m_stateManager.levelComplete)
	{
		const BossRewardHandoff handoff = m_activeBoss->BuildRewardHandoff(BuildBossContext());
		CompleteEncounterVictory(handoff.healPage, handoff.cutsceneId);
	}
}

void PlayState::ApplyBossProgressEvent(const BossProgressEvent& l_event)
{
	if (!m_activeBoss)
		return;

	const BossProgressResult result =
		m_activeBoss->ApplyProgress(l_event, BuildBossContext());
	if (!result.progressApplied)
		return;

	if (result.phaseAdvanced)
	{
		m_stateManager.GetAudio().PlaySound("phase_advance");
		m_phaseAnnouncementText = "Phase " + std::to_string(result.phaseIndex + 1);
		m_announcementDuration = 0.8f;
		m_phaseAnnouncementTimer = 0.8f;
		m_announcementCharSize = 22;
	}

	if (result.defeated)
		m_encounterPhase = EncounterPhase::BossReward;
}

void PlayState::CompleteEncounterVictory(bool l_healPage, const std::string& l_cutsceneId,
										 bool l_bossEncounterSkipped)
{
	m_activeBoss.reset();
	m_bossEncounterStartTime = 0.0f;
	m_world.ClearBossArenaMode();
	m_world.Borders(m_stateManager.GetWindow());

	m_stateManager.score += 1000;
	m_stateManager.GetAudio().PlaySound("level_complete");

	const int stars = CalculateStars();
	if (HasBossEncounter() && StartsBossOnStageClear() && !l_bossEncounterSkipped)
		m_stateManager.RecordBossDefeat(
			m_stateManager.currentLevel, m_stateManager.score, stars, l_healPage);
	else
		m_stateManager.RecordLevelCompletion(
			m_stateManager.currentLevel, m_stateManager.score, stars, l_healPage);

	if (m_levelConfig.abilityReward != AbilityId::None &&
		(!HasBossEncounter() || !StartsBossOnStageClear() || l_healPage))
	{
		m_abilityController.Unlock(m_levelConfig.abilityReward);
		SyncAbilityState();
	}

	if (!l_cutsceneId.empty())
	{
		m_stateManager.cutsceneId = l_cutsceneId;
		m_stateManager.cutsceneReturnState = StateType::StageSelect;
	}

	m_stateManager.levelComplete = true;

	if (m_levelConfig.id == 10)
	{
		m_levelCompleteDelay = 2.5f;
		m_stateManager.GetAudio().StopMusic();
		m_phaseAnnouncementText = "...";
		m_announcementDuration = 2.0f;
		m_phaseAnnouncementTimer = 2.0f;
		m_announcementCharSize = 36;
	}
	else
	{
		m_levelCompleteDelay = 0.5f;
	}

	m_stateManager.GetStats().OnLevelComplete(
		m_stateManager.currentLevel, m_gameTime, m_stateManager.score);

	AchievementContext ctx{};
	ctx.levelId = m_stateManager.currentLevel;
	ctx.score = m_stateManager.score;
	ctx.levelTime = m_gameTime;
	ctx.selfCollisions = m_stateManager.selfCollisions;
	ctx.applesEaten = m_applesEaten;
	ctx.applesToWin = m_levelConfig.applesToWin;
	ctx.predatorApplesEaten = m_predatorApplesEaten;
	ctx.quicksandTouches = m_quicksandTouches;
	ctx.timedAppleMisses = m_timedAppleMisses;
	ctx.screenFlipped = m_screenFlipped;
	ctx.screenFlipStartTime = m_screenFlipStartTime;
	ctx.reachedMinBodyFromCollision = m_reachedMinBody;
	ctx.stats = &m_stateManager.GetStats().GetStats();
	ctx.starRatings = m_stateManager.starRatings;
	ctx.completedLevelCount = m_stateManager.GetCompletedLevelCount();
	m_stateManager.GetAchievements().OnLevelComplete(ctx);
	m_stateManager.GetAchievements().OnStatsUpdate(ctx);
}

void PlayState::HandleInput()
{
	Window& window = m_stateManager.GetWindow();

	// Debounced Pause (Escape)
	if (window.IsKeyPressed(sf::Keyboard::Escape))
	{
		if (m_escReleased)
		{
			m_escReleased = false;
			m_stateManager.PushState(StateType::Pause);
			return;
		}
	}
	else
	{
		m_escReleased = true;
	}

	// Debounced Quick restart (R)
	if (window.IsKeyPressed(sf::Keyboard::R))
	{
		if (m_rReleased)
		{
			m_rReleased = false;
			OnEnter(); // restart level
			return;
		}
	}
	else
	{
		m_rReleased = true;
	}

	bool canUseAbilities = !m_snake.HasLost() && m_levelCompleteDelay < 0.0f &&
		m_encounterPhase != EncounterPhase::BossTransition &&
		m_encounterPhase != EncounterPhase::BossReward;

	if (window.IsKeyPressed(sf::Keyboard::Q))
	{
		if (m_cycleReleased && canUseAbilities)
		{
			m_cycleReleased = false;
			if (m_abilityController.CycleEquipped(1))
				m_stateManager.equippedAbility = m_abilityController.GetEquipped();
		}
	}
	else
	{
		m_cycleReleased = true;
	}

	if (window.IsKeyPressed(sf::Keyboard::Space))
	{
		if (m_activateReleased && canUseAbilities)
		{
			m_activateReleased = false;
			if (m_abilityController.TryActivateEquipped())
			{
				if (m_activeBoss)
				{
					BossProgressEvent event{};
					event.type = BossProgressEventType::AbilityActivated;
					event.interactionType = BossInteractionType::CounterHit;
					event.ability = m_abilityController.GetActive();
					event.amount = 1;
					event.gridX = m_snake.GetPosition().x;
					event.gridY = m_snake.GetPosition().y;
					ApplyBossProgressEvent(event);
				}
				SyncAbilityState();
			}
		}
	}
	else
	{
		m_activateReleased = true;
	}

	// Determine desired direction from input
	Direction inputDir = Direction::None;
	if (window.IsKeyPressed(sf::Keyboard::Up) || window.IsKeyPressed(sf::Keyboard::W))
		inputDir = Direction::Up;
	else if (window.IsKeyPressed(sf::Keyboard::Down) || window.IsKeyPressed(sf::Keyboard::S))
		inputDir = Direction::Down;
	else if (window.IsKeyPressed(sf::Keyboard::Right) || window.IsKeyPressed(sf::Keyboard::D))
		inputDir = Direction::Right;
	else if (window.IsKeyPressed(sf::Keyboard::Left) || window.IsKeyPressed(sf::Keyboard::A))
		inputDir = Direction::Left;

	// Poison: invert controls when active
	if (inputDir != Direction::None && m_levelConfig.hasPoisonApples && m_poisonApple.IsControlInverted())
	{
		switch (inputDir)
		{
			case Direction::Up:    inputDir = Direction::Down;  break;
			case Direction::Down:  inputDir = Direction::Up;    break;
			case Direction::Left:  inputDir = Direction::Right; break;
			case Direction::Right: inputDir = Direction::Left;  break;
			default: break;
		}
	}

	// Control shuffle: remap direction
	if (inputDir != Direction::None && m_levelConfig.hasControlShuffle)
		inputDir = m_controlShuffle.MapDirection(inputDir);

	// Apply direction (prevent 180-degree reversal)
	if (inputDir != Direction::None)
	{
		Direction cur = m_snake.GetDirection();
		bool valid = true;
		if (inputDir == Direction::Up && cur == Direction::Down) valid = false;
		if (inputDir == Direction::Down && cur == Direction::Up) valid = false;
		if (inputDir == Direction::Left && cur == Direction::Right) valid = false;
		if (inputDir == Direction::Right && cur == Direction::Left) valid = false;
		if (valid)
			m_snake.SetDirection(inputDir);
	}

	// Cheat code
	if (window.IsKeyPressed(sf::Keyboard::E))
		m_cheatExtend = true;
}

void PlayState::Update(float l_dt)
{
	m_elapsedTime += l_dt;
	m_gameTime += l_dt;
	m_stateManager.GetStats().UpdatePlaytime(l_dt);
	m_abilityController.Update(l_dt);
	m_snake.UpdateVisuals(l_dt);
	m_postProcessor.Update(l_dt);

	// Poll achievement notifications
	{
		AchievementManager& achMgr = m_stateManager.GetAchievements();
		while (achMgr.HasPendingNotification())
		{
			AchievementId id = achMgr.PopNotification();
			auto allAch = GetAllAchievements();
			int idx = static_cast<int>(id);
			if (idx >= 0 && idx < (int)allAch.size())
				m_achievementNotif.Push(allAch[idx]);
		}
		m_achievementNotif.Update(l_dt);
	}

	// Endless mode: mechanic cycling
	if (m_endlessCtrl)
	{
		auto event = m_endlessCtrl->Update(l_dt);

		// Show warning text
		if (!event.warningText.empty())
		{
			m_endlessWarningText = event.warningText;
			m_endlessWarningTimer = 2.5f;
			m_stateManager.GetAudio().PlaySound("endless_warning");
		}

		if (m_endlessWarningTimer > 0.0f)
			m_endlessWarningTimer -= l_dt;

		if (event.changed)
		{
			// Deactivate old mechanic
			if (event.deactivatedMechanic >= 0)
			{
				switch (event.deactivatedMechanic)
				{
					case EndlessModeController::MECH_BLACKOUTS:
						m_levelConfig.hasBlackouts = false; break;
					case EndlessModeController::MECH_QUICKSAND:
						m_levelConfig.hasQuicksand = false; break;
					case EndlessModeController::MECH_MIRROR_GHOST:
						m_levelConfig.hasMirrorGhost = false; break;
					case EndlessModeController::MECH_TIMED_APPLES:
						m_levelConfig.hasTimedApples = false; break;
					case EndlessModeController::MECH_POISON_APPLES:
						m_levelConfig.hasPoisonApples = false; break;
					case EndlessModeController::MECH_EARTHQUAKES:
						m_levelConfig.hasEarthquakes = false; break;
					case EndlessModeController::MECH_PREDATOR:
						m_levelConfig.hasPredator = false; break;
					case EndlessModeController::MECH_CONTROL_SHUFFLE:
						m_levelConfig.hasControlShuffle = false; break;
					default: break;
				}
			}

			// Activate new mechanic
			if (event.activatedMechanic >= 0)
			{
				switch (event.activatedMechanic)
				{
					case EndlessModeController::MECH_BLACKOUTS:
						m_levelConfig.hasBlackouts = true;
						m_blackout.Reset();
						break;
					case EndlessModeController::MECH_QUICKSAND:
					{
						m_levelConfig.hasQuicksand = true;
						float maxThick = std::max({m_world.GetEffectiveThickness(0),
							m_world.GetEffectiveThickness(1), m_world.GetEffectiveThickness(2),
							m_world.GetEffectiveThickness(3)});
						m_quicksand.Reset(m_world.GetMaxX(), m_world.GetMaxY(),
							maxThick, m_snake.GetBlockSize(), m_world.GetTopOffset());
						break;
					}
					case EndlessModeController::MECH_MIRROR_GHOST:
						m_levelConfig.hasMirrorGhost = true;
						m_mirrorGhost.Reset();
						break;
					case EndlessModeController::MECH_TIMED_APPLES:
						m_levelConfig.hasTimedApples = true;
						m_timedApple.Reset(6.0f);
						break;
					case EndlessModeController::MECH_POISON_APPLES:
						m_levelConfig.hasPoisonApples = true;
						m_poisonApple.Reset(m_snake.GetBlockSize());
						m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());
						break;
					case EndlessModeController::MECH_EARTHQUAKES:
						m_levelConfig.hasEarthquakes = true;
						break;
					case EndlessModeController::MECH_PREDATOR:
						m_levelConfig.hasPredator = true;
						m_predator.Reset(m_snake.GetBlockSize(), m_snake, m_world);
						m_predatorApplesEaten = 0;
						break;
					case EndlessModeController::MECH_CONTROL_SHUFFLE:
						m_levelConfig.hasControlShuffle = true;
						m_controlShuffle.Reset();
						break;
					default: break;
				}
				m_stateManager.GetAudio().PlaySound("endless_cycle");
				m_screenShake.Trigger(0.3f, 3.0f);
			}

			// Apply speed multiplier
			float baseSpeed = 10.0f + (m_applesEaten / 5) * 0.5f;
			m_snake.SetSpeed(baseSpeed * m_endlessCtrl->GetSpeedMultiplier());

			// Update visual corruption
			m_snake.SetCorruption(m_endlessCtrl->GetCorruption());
		}
	}

	// Transition animation timers
	if (m_pageTurnTimer > 0.0f)
		m_pageTurnTimer -= l_dt;
	if (m_deathInkRunActive)
	{
		m_deathInkRunTimer += l_dt;
		if (m_deathInkRunTimer >= m_deathInkRunDuration)
		{
			m_deathInkRunActive = false;
			m_stateManager.SwitchTo(StateType::GameOver);
		}
		m_particles.Update(l_dt);
		return;
	}
	if (m_appleBurstTimer > 0.0f)
		m_appleBurstTimer -= l_dt;
	if (m_borderHatchTimer > 0.0f)
		m_borderHatchTimer -= l_dt;

	float speed = m_levelConfig.baseSpeed;
	// Speed creep: +0.5 every 5 apples (noticeable step)
	speed += (m_applesEaten / 5) * 0.5f;
	// Phantom rule: continuous micro-creep (+0.03/apple, imperceptible individually)
	speed += m_applesEaten * 0.03f;
	speed *= m_speedModifier;

	float timeStep = 1.0f / speed;

	if (m_elapsedTime >= timeStep && m_levelCompleteDelay < 0.0f && !IsBossRewardQuiesced())
	{
		Window& window = m_stateManager.GetWindow();

		// Capture head position before tick for accurate VFX placement
		Position preTickHead = m_snake.GetPosition();

		m_world.Update(window, m_snake);
		m_snake.Tick(window.GetWindowSize());

		// Wall collision grace: forgive wall death right after a control shuffle
		if (m_snake.HasLost())
		{
			if (m_levelConfig.hasControlShuffle && m_controlShuffle.IsGracePeriod())
				m_snake.LoseStatus(false);
			else
			{
				OnDeath();
				return;
			}
		}

		// Detect apple eaten by comparing count
		int newApplesEaten = m_world.GetApplesEaten();
		if (newApplesEaten > m_applesEaten)
		{
			m_applesEaten = newApplesEaten;
			OnAppleEaten(preTickHead);
		}

		const bool skipRestOfTick = (m_encounterPhase == EncounterPhase::BossReward);
		if (!skipRestOfTick)
		{
		// Detect world shrink and award bonus
		int newShrinkCount = m_world.GetShrinkCount();
		if (newShrinkCount > m_lastShrinkCount)
		{
			m_stateManager.score += 250; // survive world shrink bonus
			m_lastShrinkCount = newShrinkCount;
			m_stateManager.GetAudio().PlaySound("world_shrink");
			m_screenShake.Trigger(0.3f, 3.0f);
			m_world.FlashBorders(0.2f);
			m_borderHatchTimer = m_borderHatchDuration; // Trigger hatch fill animation
		}

		// Phantom rule: border color degradation toward danger-red
		if (newShrinkCount > 0 && m_levelConfig.id != 10 && m_world.GetFlashTimer() <= 0.0f)
		{
			float t = std::min(1.0f, newShrinkCount / 8.0f);
			sf::Color base = m_levelConfig.border;
			sf::Color danger(140, 30, 20);
			sf::Color degraded(
				(sf::Uint8)(base.r + t * ((int)danger.r - (int)base.r)),
				(sf::Uint8)(base.g + t * ((int)danger.g - (int)base.g)),
				(sf::Uint8)(base.b + t * ((int)danger.b - (int)base.b)));
			m_world.SetBorderColor(degraded);
		}

		// Check for self-collision
		if (m_snake.DidSelfCollide())
		{
			int segmentsLost = (int)m_snake.GetLastCutSegments().size();
			m_stateManager.selfCollisions++;
			m_stateManager.score = std::max(0, m_stateManager.score - 50);
			m_hud.FlashScore();
			UpdateCombo(true);
			m_stateManager.GetAudio().PlaySound("self_collide");
			m_particles.SpawnSelfCollisionCut(m_snake.GetLastCutSegments(), m_snake.GetBlockSize(),
										 m_levelConfig.snakeBody);
			m_snake.ClearSelfCollideFlag();
			m_stateManager.GetStats().OnSelfCollision(segmentsLost);

			// Track if body was reduced to 1 segment (for Ouroboros achievement)
			if (m_snake.GetBodySize() <= 1)
				m_reachedMinBody = true;

			// Achievement check
			{
				AchievementContext ctx{};
				ctx.stats = &m_stateManager.GetStats().GetStats();
				m_stateManager.GetAchievements().OnSelfCollision(ctx);
			}
		}

		// Mirror ghost update (tick-based, same rate as snake movement)
		if (m_levelConfig.hasMirrorGhost)
		{
			float bs = m_snake.GetBlockSize();
			float centerX = (m_world.GetEffectiveThickness(3) / bs +
							 m_world.GetMaxX() - m_world.GetEffectiveThickness(1) / bs - 1) / 2.0f;
			float centerY = ((m_world.GetEffectiveThickness(0) + m_world.GetTopOffset()) / bs +
							 m_world.GetMaxY() - m_world.GetEffectiveThickness(2) / bs - 1) / 2.0f;
			m_mirrorGhost.Update(m_snake, centerX, centerY);

			if (m_mirrorGhost.CheckCollision(m_snake.GetPosition()))
			{
				m_stateManager.deathCtx.cause = StateManager::DeathCause::MirrorGhost;
				m_snake.LoseStatus(true);
			}
		}

		// Check poison apple collision
		if (m_levelConfig.hasPoisonApples)
		{
			if (m_poisonApple.CheckCollision(m_snake.GetPosition()))
			{
				m_poisonApple.OnPoisonEaten();
				m_stateManager.score = std::max(0, m_stateManager.score - 200);
				m_hud.FlashScore();
				UpdateCombo(true);

				sf::Vector2f poisonPixelPos = m_poisonApple.GetPixelPos(m_snake.GetBlockSize());
				m_particles.SpawnAppleBurst(poisonPixelPos, sf::Color(
					std::min(255, (int)m_levelConfig.inkTint.r + 80),
					m_levelConfig.inkTint.g,
					std::min(255, (int)m_levelConfig.inkTint.b + 30)));
				m_particles.SpawnFloatingText("-200", poisonPixelPos, sf::Color(180, 40, 50));
				m_stateManager.GetAudio().PlaySound("self_collide");

				for (int i = 0; i < m_poisonApple.GetGrowAmount(); i++)
					m_snake.Extend();

				m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());

				m_poisonApplesThisLevel++;
				m_stateManager.GetStats().OnPoisonAppleEaten();

				// Achievement check
				{
					AchievementContext ctx{};
					ctx.poisonApplesThisLevel = m_poisonApplesThisLevel;
					ctx.stats = &m_stateManager.GetStats().GetStats();
					m_stateManager.GetAchievements().OnPoisonAppleEaten(ctx);
				}
			}
		}

		// Check predator collision with player
		if (m_levelConfig.hasPredator)
		{
			if (m_predator.HitPlayer(m_snake.GetPosition()))
			{
				m_stateManager.deathCtx.cause = StateManager::DeathCause::Predator;
				m_snake.LoseStatus(true);
				m_stateManager.GetStats().OnPredatorKilledPlayer();
			}
		}

		// Check death (entity collisions — not forgiven by grace)
		if (m_snake.HasLost())
		{
			OnDeath();
			return;
		}

		// Cheat code
		if (m_cheatExtend)
		{
			m_snake.Extend();
			m_cheatExtend = false;
		}

		} // !skipRestOfTick

		m_elapsedTime -= timeStep;
	}

	// Update HUD
	m_stateManager.levelTime = m_gameTime;
	if (m_endlessCtrl)
	{
		// Endless mode: show "Endless Mode" and survival time, no apple target
		m_hud.Update(m_stateManager.score, m_stateManager.comboMultiplier,
					 m_applesEaten, 0,
					 "Endless Mode", m_endlessCtrl->GetSurvivalTime(), l_dt,
					 m_levelConfig.hasPredator ? m_predatorApplesEaten : -1);
	}
	else
	{
		m_hud.Update(m_stateManager.score, m_stateManager.comboMultiplier,
					 m_applesEaten, m_levelConfig.applesToWin,
					 m_levelConfig.name, m_gameTime, l_dt,
					 m_levelConfig.hasPredator ? m_predatorApplesEaten : -1);
	}
	m_abilityHud.Update(m_abilityController);

	// Update visual effects (continuous, not tick-based)
	m_particles.Update(l_dt);
	m_screenShake.Update(l_dt, m_stateManager.GetWindow());
	m_world.UpdateFlash(l_dt);

	// --- Level mechanic continuous updates ---

	// Reset speed modifier each frame; mechanics below accumulate into it
	m_speedModifier = 1.0f;

	if (m_activeBoss && !m_stateManager.endlessMode)
	{
		const BossArenaRequirements& arena = m_activeBoss->GetArenaRequirements();
		if (arena.hazardIntensityMode == BossHazardIntensityMode::BossEscalation)
			m_speedModifier *= 1.1f;
		// BossHazardIntensityMode::Disabled / Unchanged: no extra modifier here yet
	}

	// Blackout (Level 2)
	if (m_levelConfig.hasBlackouts)
	{
		m_blackout.Update(l_dt, m_snake);
		if (m_blackout.JustStartedBlackout())
		{
			m_world.RespawnApple(m_snake);
			m_stateManager.GetAudio().PlaySound("blackout_on");
		}
	}

	// Quicksand speed modifier (Level 3)
	if (m_levelConfig.hasQuicksand)
	{
		float maxThick = std::max({m_world.GetEffectiveThickness(0), m_world.GetEffectiveThickness(1),
								   m_world.GetEffectiveThickness(2), m_world.GetEffectiveThickness(3)});
		m_quicksand.Update(l_dt, m_world.GetMaxX(), m_world.GetMaxY(),
						   maxThick, m_snake.GetBlockSize(),
						   m_world.GetTopOffset());

		bool onQuicksand = m_quicksand.IsOnQuicksand(m_snake.GetPosition());
		if (onQuicksand)
		{
			m_speedModifier *= 0.5f;
			if (!m_wasOnQuicksand)
				m_quicksandTouches++;
		}
		m_wasOnQuicksand = onQuicksand;
	}

	// Timer-based world shrinking (Level 3)
	if (m_levelConfig.shrinkTimerSec > 0.0f && m_levelCompleteDelay < 0.0f &&
		!IsBossRewardQuiesced())
	{
		Window& window = m_stateManager.GetWindow();
		m_world.UpdateTimedShrink(l_dt, window, m_snake);
		// Shrink may have moved borders onto the snake
		m_world.CheckCollision(window, m_snake);
		if (m_snake.HasLost()) { OnDeath(); return; }
	}

	// Poison apples (Level 6)
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.Update(l_dt);
		if (m_poisonApple.GetSpeedMultiplier() > 1.0f)
			m_speedModifier *= m_poisonApple.GetSpeedMultiplier();

		// Snake color flash when poisoned (driven from Update, not Render)
		if (m_poisonApple.IsControlInverted())
		{
			float flash = std::sin(m_gameTime * 10.0f);
			if (flash > 0)
				m_snake.SetColors(
					sf::Color(m_levelConfig.snakeHead.r, std::min(255, (int)m_levelConfig.snakeHead.g + 40), m_levelConfig.snakeHead.b),
					sf::Color(std::min(255, (int)m_levelConfig.snakeBody.r + 30), m_levelConfig.snakeBody.g, std::min(255, (int)m_levelConfig.snakeBody.b + 30)));
			else
				m_snake.SetColors(m_levelConfig.snakeHead, m_levelConfig.snakeBody);
		}
		else
		{
			m_snake.SetColors(m_levelConfig.snakeHead, m_levelConfig.snakeBody);
		}
	}

	// Timed apples (Level 5)
	if (m_levelConfig.hasTimedApples && m_levelCompleteDelay < 0.0f &&
		!IsBossRewardQuiesced())
	{
		m_timedApple.Update(l_dt);

		if (m_timedApple.HasExpired())
		{
			// Don't penalize if the snake is already on the apple (will be
			// eaten on the next tick — timer expired between ticks)
			sf::Vector2f ap = m_world.GetApplePos();
			Position head = m_snake.GetPosition();
			if (head.x == (int)ap.x && head.y == (int)ap.y)
			{
				m_timedApple.OnAppleEaten(GetAppleTimerDuration());
			}
			else
			{
				// Penalty: apple missed, world shrinks
				m_timedAppleMisses++;
				Window& window = m_stateManager.GetWindow();
				m_world.TriggerShrink(window, m_snake);
				m_lastShrinkCount = m_world.GetShrinkCount();
				m_stateManager.GetAudio().PlaySound("apple_miss");
				m_screenShake.Trigger(0.3f, 3.0f);
				m_world.FlashBorders(0.2f);

				// Check if shrink crushed the snake
				m_world.CheckCollision(window, m_snake);
				if (m_snake.HasLost()) { OnDeath(); return; }

				// Respawn apple with adjusted timer
				m_world.RespawnApple(m_snake);
				m_timedApple.OnAppleEaten(GetAppleTimerDuration());
			}
		}
	}

	// Earthquake (Level 7)
	if (m_levelConfig.hasEarthquakes && m_levelCompleteDelay < 0.0f &&
		!IsBossRewardQuiesced())
	{
		Window& window = m_stateManager.GetWindow();
		m_earthquake.Update(l_dt, m_world, window);

		if (m_earthquake.IsWarning())
			m_screenShake.Trigger(0.1f, 1.5f);

		if (m_earthquake.JustQuaked())
		{
			m_screenShake.Trigger(0.6f, 6.0f);
			m_stateManager.GetAudio().PlaySound("earthquake");
			m_world.FlashBorders(0.3f);

			// Only respawn apple/poison if they're now inside a wall
			if (!m_world.IsAppleInBounds(m_snake.GetBlockSize()))
				m_world.RespawnApple(m_snake);

			if (m_levelConfig.hasPoisonApples && !m_poisonApple.IsInBounds(m_world, m_snake.GetBlockSize()))
				m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());

			m_world.CheckCollision(window, m_snake);
			if (m_snake.HasLost()) { OnDeath(); return; }
		}
	}

	// Predator (Level 8)
	if (m_levelConfig.hasPredator && m_levelCompleteDelay < 0.0f &&
		!IsBossRewardQuiesced())
	{
		m_predator.Update(l_dt, m_world, m_snake);

		// Check if predator moved onto player head
		if (m_predator.HitPlayer(m_snake.GetPosition()))
		{
			m_stateManager.deathCtx.cause = StateManager::DeathCause::Predator;
			m_snake.LoseStatus(true);
			m_stateManager.GetStats().OnPredatorKilledPlayer();
			OnDeath();
			return;
		}

		if (m_predator.JustAteApple())
		{
			m_predatorApplesEaten++;
			m_stateManager.GetStats().OnPredatorAteApple();

			// World shrinks when predator eats apple
			Window& window = m_stateManager.GetWindow();
			m_world.TriggerShrink(window, m_snake);
			m_lastShrinkCount = m_world.GetShrinkCount();
			m_stateManager.GetAudio().PlaySound("predator_eat");
			m_screenShake.Trigger(0.3f, 3.0f);
			m_world.FlashBorders(0.2f);

			// Score penalty
			m_stateManager.score = std::max(0, m_stateManager.score - 150);
			m_hud.FlashScore();
			sf::Vector2f ap(m_world.GetApplePos().x * m_snake.GetBlockSize(),
							m_world.GetApplePos().y * m_snake.GetBlockSize());
			m_particles.SpawnFloatingText("-150", ap, sf::Color(70, 80, 150));

			// Respawn apple
			m_world.RespawnApple(m_snake);

			// Lose condition: predator ate 5 apples
			if (m_predatorApplesEaten >= 5)
				m_snake.LoseStatus(true);

			// Check if shrink crushed the snake
			m_world.CheckCollision(window, m_snake);
			if (m_snake.HasLost()) { OnDeath(); return; }
		}

		if (m_predator.JustStartedHunting())
		{
			m_stateManager.GetAudio().PlaySound("predator_hunt");
			m_screenShake.Trigger(0.5f, 4.0f);
		}
	}

	// Control shuffle (Level 9)
	if (m_levelConfig.hasControlShuffle && m_levelCompleteDelay < 0.0f)
	{
		m_controlShuffle.Update(l_dt);

		if (m_controlShuffle.IsWarning())
			m_stateManager.GetAudio().PlaySound("shuffle_warning");

		if (m_controlShuffle.JustShuffled())
		{
			m_stateManager.GetAudio().PlaySound("control_shuffle");
			m_screenShake.Trigger(0.3f, 3.0f);
		}
	}

	// Border pulse during grace period; restore degraded/base border when grace ends
	if (m_levelConfig.hasControlShuffle && m_controlShuffle.IsGracePeriod())
	{
		float pulse = std::sin(m_gameTime * 20.0f);
		sf::Uint8 g = (sf::Uint8)(200 + 55 * pulse);
		m_world.SetBorderColor(sf::Color(
			m_levelConfig.accentColor.r,
			(sf::Uint8)std::max(0, std::min(255, (int)m_levelConfig.accentColor.g + (int)(g - 200))),
			(sf::Uint8)std::min(255, (int)m_levelConfig.accentColor.b + 50)));
		m_wasInGracePeriod = true;
	}
	else if (m_wasInGracePeriod)
	{
		// Grace just ended — restore border (degraded if shrinks occurred, base otherwise)
		m_world.SetBorderColor(m_levelConfig.border);
		m_wasInGracePeriod = false;
	}

	// Psychedelic color cycling (Level 9 theme)
	if (m_levelConfig.id == 9 && m_levelCompleteDelay < 0.0f)
	{
		m_psychedelicTimer += l_dt;

		// Background tint cycles between purple and teal (applied as overlay in Render)
		// Apple: RGB cycling via phase-shifted sin waves
		float ap = m_psychedelicTimer * 2.0f;
		m_world.SetAppleColor(sf::Color(
			(sf::Uint8)(100 + 80 * std::sin(ap)),
			(sf::Uint8)(100 + 80 * std::sin(ap + 2.094f)),
			(sf::Uint8)(100 + 80 * std::sin(ap + 4.189f))));
	}

	// Heartbeat as borders tighten
	{
		float borderFrac = (m_world.GetBorderThickness() * 2.0f) /
			(float)m_stateManager.GetWindow().GetWindowSize().x;
		if (borderFrac > 0.35f && m_levelCompleteDelay < 0.0f)
		{
			float interval = 1.2f - (borderFrac - 0.35f) * 1.5f;
			interval = std::max(0.4f, interval);
			m_heartbeatTimer -= l_dt;
			if (m_heartbeatTimer <= 0.0f)
			{
				m_stateManager.GetAudio().PlaySound("heartbeat");
				m_heartbeatTimer = interval;
			}
		}
		else
		{
			m_heartbeatTimer = 0.0f;
		}
	}

	if (m_activeBoss)
		UpdateBossEncounter(l_dt);

	SyncAbilityState();

	// Announcement timer (all levels — generalized from L10-only)
	if (m_phaseAnnouncementTimer > 0.0f)
		m_phaseAnnouncementTimer -= l_dt;

	// Deferred level-complete transition (lets particles render first)
	if (m_levelCompleteDelay >= 0.0f)
	{
		m_levelCompleteDelay -= l_dt;
		if (m_levelCompleteDelay < 0.0f)
			m_stateManager.SwitchTo(StateType::GameOver);
	}
}

void PlayState::Render()
{
	Window& window = m_stateManager.GetWindow();
	bool usePostProcess = m_postProcessor.IsAvailable();

	// Begin post-processing capture (game scene renders to offscreen RT)
	if (usePostProcess)
	{
		m_postProcessor.Begin();

		// Propagate screen shake view (offset + rotation) from window to the RT
		// so shake effects are visible in the post-processed output
		sf::View shakeView = window.GetRenderWindow().getView();
		m_postProcessor.GetTarget().setView(shakeView);
	}

	sf::RenderTarget& target = usePostProcess
		? m_postProcessor.GetTarget()
		: (sf::RenderTarget&)window.GetRenderWindow();

	// Level 10: screen flip (The Cruel Twist at apple 19)
	sf::View savedView;
	if (m_screenFlipped)
	{
		if (usePostProcess)
		{
			savedView = m_postProcessor.GetTarget().getView();
			sf::View flipped = savedView;
			flipped.setRotation(savedView.getRotation() + 180.f);
			m_postProcessor.GetTarget().setView(flipped);
		}
		else
		{
			savedView = window.GetRenderWindow().getView();
			sf::View flipped = savedView;
			flipped.setRotation(savedView.getRotation() + 180.f);
			window.SetView(flipped);
		}
	}

	// Draw paper background with page-turn entry animation
	if (m_paperBackground.IsGenerated())
	{
		if (m_pageTurnTimer > 0.0f)
		{
			// Page slides in from the right
			float progress = m_pageTurnTimer / m_pageTurnDuration; // 1.0 → 0.0
			float slideOffset = progress * (float)window.GetWindowSize().x;

			// Save view, offset for slide
			sf::View slideView = target.getView();
			sf::View offsetView = slideView;
			offsetView.move(slideOffset, 0);
			target.setView(offsetView);
			m_paperBackground.Render(target);
			target.setView(slideView);
		}
		else
		{
			m_paperBackground.Render(target);
		}
	}

	// Level 9: Psychedelic tint overlay cycling on paper background
	if (m_levelConfig.id == 9 && m_psychedelicTimer > 0.0f)
	{
		float t = (std::sin(m_psychedelicTimer * 0.8f) + 1.0f) / 2.0f;
		sf::Uint8 r = (sf::Uint8)(80 + t * 60);
		sf::Uint8 g = (sf::Uint8)(30 + (1.0f - t) * 60);
		sf::Uint8 b = (sf::Uint8)(100 + t * 30);
		sf::RectangleShape psychOverlay(sf::Vector2f(
			(float)window.GetWindowSize().x, (float)window.GetWindowSize().y));
		psychOverlay.setFillColor(sf::Color(r, g, b, 40)); // Subtle tint wash
		target.draw(psychOverlay);
	}

	m_world.RenderInk(target, m_gameTime);

	if (m_levelConfig.hasEarthquakes)
		m_earthquake.RenderTo(target, m_world);

	if (m_levelConfig.hasQuicksand)
		m_quicksand.RenderTo(target, m_snake.GetBlockSize());

	if (m_levelConfig.hasTimedApples)
	{
		sf::Vector2f applePixelPos(
			m_world.GetApplePos().x * m_snake.GetBlockSize(),
			m_world.GetApplePos().y * m_snake.GetBlockSize());
		m_timedApple.RenderTo(target, applePixelPos, m_snake.GetBlockSize() / 2.0f);
	}

	if (m_levelConfig.hasMirrorGhost)
	{
		float bs = m_snake.GetBlockSize();
		int bMinX = (int)(m_world.GetEffectiveThickness(3) / bs);
		int bMaxX = (int)(m_world.GetMaxX() - m_world.GetEffectiveThickness(1) / bs - 1);
		int bMinY = (int)((m_world.GetEffectiveThickness(0) + m_world.GetTopOffset()) / bs);
		int bMaxY = (int)(m_world.GetMaxY() - m_world.GetEffectiveThickness(2) / bs - 1);
		m_mirrorGhost.RenderTo(target, bs, bMinX, bMaxX, bMinY, bMaxY);
	}

	// Poison apple rendering + Phase 2 real apple pulse
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.RenderTo(target, m_snake.GetBlockSize());

		// In Phase 2, make the real apple pulse too
		if (m_poisonApple.GetRealApplesEaten() >= 8)
		{
			float pulseRadius = (m_snake.GetBlockSize() / 2.0f) + std::sin(m_gameTime * 4.0f) * 1.0f;
			float baseRadius = m_snake.GetBlockSize() / 2.0f;
			m_realPulse.setRadius(pulseRadius);
			m_realPulse.setOrigin(pulseRadius - baseRadius, pulseRadius - baseRadius);
			m_realPulse.setFillColor(m_levelConfig.apple);
			m_realPulse.setPosition(m_world.GetApplePos().x * m_snake.GetBlockSize(),
									m_world.GetApplePos().y * m_snake.GetBlockSize());
			target.draw(m_realPulse);
		}
	}

	if (m_levelConfig.hasPredator)
		m_predator.RenderTo(target, m_snake.GetBlockSize());

	if (m_activeBoss)
		m_activeBoss->RenderTo(target, m_gameTime, BuildBossContext());

	// Snake: render with ink style to the post-process target
	m_snake.RenderInk(target);

	// Apple burst outline effect (expanding circle on eat)
	if (m_appleBurstTimer > 0.0f)
	{
		float progress = 1.0f - (m_appleBurstTimer / kAppleBurstDuration); // 0→1
		float burstRadius = m_snake.GetBlockSize() * (1.0f + progress * 1.5f);
		sf::Uint8 burstAlpha = (sf::Uint8)(180 * (1.0f - progress));
		sf::Color burstOutline(m_appleBurstColor.r, m_appleBurstColor.g,
							   m_appleBurstColor.b, burstAlpha);
		float cx = m_appleBurstPos.x + m_snake.GetBlockSize() * 0.5f;
		float cy = m_appleBurstPos.y + m_snake.GetBlockSize() * 0.5f;
		InkRenderer::DrawWobblyCircle(target, cx, cy, burstRadius,
									  sf::Color::Transparent, burstOutline,
									  1.5f, m_levelConfig.corruption * 0.5f,
									  (unsigned int)(m_gameTime * 100.0f), 12);
	}

	// Border hatch fill animation (rapid strokes appearing in new border area)
	if (m_borderHatchTimer > 0.0f)
	{
		float progress = 1.0f - (m_borderHatchTimer / m_borderHatchDuration); // 0→1
		int strokeCount = (int)(progress * 15);
		sf::Color hatchColor(m_levelConfig.inkTint.r, m_levelConfig.inkTint.g,
							 m_levelConfig.inkTint.b, (sf::Uint8)(100 * (1.0f - progress)));
		unsigned int seed = (unsigned int)(m_gameTime * 50.0f);

		// Build border band rects: top, right, bottom, left
		float winW = (float)window.GetWindowSize().x;
		float winH = (float)window.GetWindowSize().y;
		float topOff = m_world.GetTopOffset();
		float eTop = m_world.GetEffectiveThickness(0);
		float eRight = m_world.GetEffectiveThickness(1);
		float eBottom = m_world.GetEffectiveThickness(2);
		float eLeft = m_world.GetEffectiveThickness(3);
		sf::FloatRect bands[4] = {
			{ 0, topOff, winW, eTop },                          // top
			{ winW - eRight, topOff, eRight, winH - topOff },   // right
			{ 0, winH - eBottom, winW, eBottom },                // bottom
			{ 0, topOff, eLeft, winH - topOff }                  // left
		};

		for (int s = 0; s < strokeCount; s++)
		{
			unsigned int h = InkRenderer::Hash(seed, (unsigned int)s);
			// Pick a border band based on hash
			const sf::FloatRect& band = bands[h % 4];
			float bw = std::max(1.0f, band.width);
			float bh = std::max(1.0f, band.height);
			float sx = band.left + (float)(h % (int)bw);
			float sy = band.top + (float)((h >> 8) % (int)bh);
			float len = 8.0f + (float)((h >> 16) % 12);
			bool horiz = (h >> 28) & 1;
			if (horiz)
				InkRenderer::DrawWobblyLine(target, sx, sy, sx + len, sy,
											hatchColor, 1.0f, 0.3f, h);
			else
				InkRenderer::DrawWobblyLine(target, sx, sy, sx, sy + len,
											hatchColor, 1.0f, 0.3f, h);
		}
	}

	m_particles.RenderTo(target);

	if (m_levelConfig.hasBlackouts)
		m_blackout.RenderTo(target, window.GetWindowSize(), m_snake.GetBlockSize());

	// Restore un-flipped view for HUD and UI overlays (never upside-down)
	if (m_screenFlipped)
	{
		if (usePostProcess)
			m_postProcessor.GetTarget().setView(savedView);
		else
			window.SetView(savedView);
	}

	// End post-processing capture and apply shader chain
	if (usePostProcess)
	{
		m_postProcessor.End();

		// Reset window view to default before drawing post-processed result,
		// otherwise screen shake offset gets applied twice (once in RT, once on window)
		window.SetView(window.GetDefaultView());
		m_postProcessor.Apply(window);
	}
	else
	{
		window.SetView(window.GetDefaultView());
	}

	// HUD and overlays render directly to window (no post-processing, stays crisp)
	if (m_levelConfig.hasControlShuffle)
		m_controlShuffle.Render(window);

	m_hud.Render(window);
	m_abilityHud.Render(window);

	// Achievement notification popup
	if (m_achievementNotif.IsShowing())
	{
		sf::RenderTarget& target = window.GetRenderWindow();
		m_achievementNotif.Render(target, (float)window.GetWindowSize().x);
	}

	// Endless mode warning text
	if (m_endlessWarningTimer > 0.0f && !m_endlessWarningText.empty() && m_announcementFontLoaded)
	{
		sf::Text warningText;
		warningText.setFont(m_announcementFont);
		warningText.setString(m_endlessWarningText);
		warningText.setCharacterSize(28);
		float alpha = std::min(1.0f, m_endlessWarningTimer * 2.0f) * 255;
		warningText.setFillColor(sf::Color(180, 50, 40, (sf::Uint8)alpha));
		sf::FloatRect wb = warningText.getLocalBounds();
		warningText.setPosition((window.GetWindowSize().x - wb.width) / 2.0f,
								window.GetWindowSize().y / 2.0f - 50.0f);
		window.Draw(warningText);
	}

	// Announcement overlay (always right-side-up, on top of everything)
	if (m_phaseAnnouncementTimer > 0.0f)
	{
		window.SetView(window.GetDefaultView());
		RenderPhaseAnnouncement(window);
	}
}

void PlayState::OnAppleEaten(const Position& l_applePos)
{
	m_consecutiveApples++;
	UpdateCombo(false);

	int points = CalculatePoints(100);
	m_stateManager.score += points;
	m_stateManager.applesEaten = m_applesEaten;

	// Stats tracking
	m_stateManager.GetStats().OnAppleEaten();
	m_stateManager.GetStats().OnComboAchieved(m_consecutiveApples);

	// Achievement checks on apple eaten
	{
		AchievementContext ctx{};
		ctx.levelId = m_stateManager.currentLevel;
		ctx.comboMultiplier = m_stateManager.comboMultiplier;
		ctx.blackoutActive = m_levelConfig.hasBlackouts && m_blackout.IsBlackout();
		ctx.applesEaten = m_applesEaten;
		ctx.applesToWin = m_levelConfig.applesToWin;
		// Check predator distance to apple for AppleThief
		if (m_levelConfig.hasPredator && !m_predator.GetBody().empty())
		{
			sf::Vector2f ap = m_world.GetApplePos();
			Position ph = m_predator.GetBody()[0];
			ctx.predatorDistToApple = (float)(std::abs(ph.x - (int)ap.x) + std::abs(ph.y - (int)ap.y));
		}
		ctx.stats = &m_stateManager.GetStats().GetStats();
		m_stateManager.GetAchievements().OnAppleEaten(ctx);
	}

	// Cruel micro-moment check
	CheckCruelMoment();

	// Audio + visual feedback
	m_stateManager.GetAudio().PlaySound("apple_eat");
	sf::Vector2f applePixelPos(
		l_applePos.x * m_snake.GetBlockSize(),
		l_applePos.y * m_snake.GetBlockSize());
	m_particles.SpawnAppleBurst(applePixelPos, m_levelConfig.apple);
	m_particles.SpawnFloatingText("+" + std::to_string(points), applePixelPos,
								  m_levelConfig.accentColor);

	// Apple burst outline effect
	m_appleBurstTimer = kAppleBurstDuration;
	m_appleBurstPos = applePixelPos;
	m_appleBurstColor = m_levelConfig.apple;

	// Mirror ghost: flip axis every 5 apples
	if (m_levelConfig.hasMirrorGhost)
	{
		m_mirrorFlipCounter++;
		if (m_mirrorFlipCounter % 5 == 0)
		{
			m_mirrorGhost.FlipAxis();
			m_stateManager.GetAudio().PlaySound("mirror_flip");
			m_screenShake.Trigger(0.2f, 2.0f);
		}
	}

	// Poison apples: notify real apple eaten, respawn poison
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.OnRealAppleEaten();
		m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());
	}

	// Predator: notify that player got the apple
	if (m_levelConfig.hasPredator)
		m_predator.OnPlayerAteApple();

	// Control shuffle: update apple counter for indicator cutoff
	if (m_levelConfig.hasControlShuffle)
		m_controlShuffle.OnAppleEaten(m_applesEaten);

	// Timed apples: reset timer with adjusted duration
	if (m_levelConfig.hasTimedApples)
		m_timedApple.OnAppleEaten(GetAppleTimerDuration());

	// Level 10 "Cruel World" phase transitions
	if (m_levelConfig.id == 10)
	{
		bool phaseChanged = false;
		if (m_applesEaten == 5 && m_cruelPhase == 0)
			{ AdvanceCruelPhase(); phaseChanged = true; }
		else if (m_applesEaten == 10 && m_cruelPhase == 1)
			{ AdvanceCruelPhase(); phaseChanged = true; }
		else if (m_applesEaten == 15 && m_cruelPhase == 2)
			{ AdvanceCruelPhase(); phaseChanged = true; }

		// Re-set timed apple with new phase duration
		if (phaseChanged && m_levelConfig.hasTimedApples)
			m_timedApple.OnAppleEaten(GetAppleTimerDuration());

		// The Cruel Twist: screen flips at apple 19
		if (m_applesEaten == 19 && !m_screenFlipped)
		{
			m_screenFlipped = true;
			m_screenFlipStartTime = m_gameTime;
			m_screenShake.Trigger(0.8f, 8.0f);
			m_stateManager.GetAudio().PlaySound("phase_advance");

			// "Oops."
			m_phaseAnnouncementText = "Oops.";
			m_announcementDuration = 1.5f;
			m_phaseAnnouncementTimer = 1.5f;
			m_announcementCharSize = 28;
		}
	}

	// Level 1 "False Hope" twist: double shrink on final apple
	if (m_levelConfig.id == 1 && m_applesEaten == m_levelConfig.applesToWin)
	{
		Window& window = m_stateManager.GetWindow();
		m_world.TriggerShrink(window, m_snake);
		m_world.TriggerShrink(window, m_snake);
		m_lastShrinkCount = m_world.GetShrinkCount();
		m_stateManager.GetAudio().PlaySound("world_shrink");
		m_screenShake.Trigger(0.5f, 5.0f);
		m_world.FlashBorders(0.3f);

		// Double-shrink may have crushed the snake
		m_world.CheckCollision(window, m_snake);
		if (m_snake.HasLost()) return;
	}

	if (m_activeBoss)
	{
		BossProgressEvent event{};
		event.type = BossProgressEventType::AppleCollected;
		event.amount = 1;
		event.gridX = l_applePos.x;
		event.gridY = l_applePos.y;
		ApplyBossProgressEvent(event);
		return;
	}

	// Check level complete
	if (m_applesEaten >= m_levelConfig.applesToWin)
	{
		if (HasBossEncounter() && StartsBossOnStageClear())
		{
			BeginBossEncounter();
			return;
		}

		const bool healPage = (m_stateManager.currentLevel >= 2 &&
			m_stateManager.currentLevel <= 9);
		CompleteEncounterVictory(healPage, "");
	}
}

void PlayState::OnDeath()
{
	m_stateManager.totalDeaths++;
	m_stateManager.levelComplete = false;
	m_stateManager.GetAudio().PlaySound("wall_death");

	// Populate death context for context-sensitive taunts
	auto& dc = m_stateManager.deathCtx;
	dc.appleCount = m_applesEaten;
	dc.wasInBlackout = m_levelConfig.hasBlackouts && m_blackout.IsBlackout();
	dc.wasOnQuicksand = m_levelConfig.hasQuicksand &&
		m_quicksand.IsOnQuicksand(m_snake.GetPosition());
	dc.hadHighCombo = m_stateManager.comboMultiplier >= 2.0f;
	dc.comboLostAt = m_consecutiveApples;
	dc.sessionBestImproved = (m_applesEaten > dc.sessionBestApples);
	if (dc.sessionBestImproved)
		dc.sessionBestApples = m_applesEaten;
	// cause is set at the kill site (predator/mirror/wall); default to Wall
	if (dc.cause == StateManager::DeathCause::Unknown)
		dc.cause = StateManager::DeathCause::Wall;
	if (!m_stateManager.endlessMode)
		m_stateManager.GetStats().OnDeath(m_stateManager.currentLevel);

	// Achievement checks on death
	{
		AchievementContext ctx{};
		ctx.levelId = m_stateManager.currentLevel;
		ctx.levelTime = m_gameTime;
		ctx.totalDeaths = m_stateManager.totalDeaths;
		ctx.stats = &m_stateManager.GetStats().GetStats();
		m_stateManager.GetAchievements().OnDeath(ctx);

		// Also check stats-based achievements
		m_stateManager.GetAchievements().OnStatsUpdate(ctx);
	}

	// Endless mode: record stats
	if (m_endlessCtrl)
	{
		m_stateManager.GetStats().OnEndlessGameOver(
			m_stateManager.score, m_endlessCtrl->GetSurvivalTime());
	}

	// Trigger ink-run death effect
	m_deathInkRunActive = true;
	m_deathInkRunTimer = 0.0f;

	// Spawn ink drip particles at snake head
	float bs = m_snake.GetBlockSize();
	sf::Vector2f headPixel(m_snake.GetPosition().x * bs, m_snake.GetPosition().y * bs);
	m_particles.SpawnInkDrips(headPixel, m_levelConfig.inkTint, 8);
}

void PlayState::UpdateCombo(bool l_reset)
{
	if (l_reset)
	{
		// Combo break taunt when losing a high combo
		if (m_stateManager.comboMultiplier >= 2.0f && m_phaseAnnouncementTimer <= 0.0f)
		{
			m_phaseAnnouncementText = "Ha.";
			m_announcementDuration = 1.0f;
			m_phaseAnnouncementTimer = 1.0f;
			m_announcementCharSize = 22;
		}
		m_consecutiveApples = 0;
		m_stateManager.comboMultiplier = 1.0f;
		m_stateManager.combo = 0;
		m_comboSoundPlayed = false;
		return;
	}

	m_stateManager.combo = m_consecutiveApples;
	if (m_consecutiveApples >= 5)
		m_stateManager.comboMultiplier = 3.0f;
	else if (m_consecutiveApples >= 4)
		m_stateManager.comboMultiplier = 2.5f;
	else if (m_consecutiveApples >= 3)
		m_stateManager.comboMultiplier = 2.0f;
	else if (m_consecutiveApples >= 2)
		m_stateManager.comboMultiplier = 1.5f;
	else
		m_stateManager.comboMultiplier = 1.0f;

	// Combo pressure text (whispered floating text at key thresholds)
	if (m_consecutiveApples == 3)
	{
		float bs = m_snake.GetBlockSize();
		sf::Vector2f headPos(m_snake.GetPosition().x * bs, m_snake.GetPosition().y * bs);
		m_particles.SpawnFloatingText("Don't mess up...", headPos,
			sf::Color(m_levelConfig.inkTint.r, m_levelConfig.inkTint.g,
					  m_levelConfig.inkTint.b, 80));
	}
	else if (m_consecutiveApples == 5)
	{
		float bs = m_snake.GetBlockSize();
		sf::Vector2f headPos(m_snake.GetPosition().x * bs, m_snake.GetPosition().y * bs);
		m_particles.SpawnFloatingText("The world is watching.", headPos,
			sf::Color(m_levelConfig.inkTint.r, m_levelConfig.inkTint.g,
					  m_levelConfig.inkTint.b, 80));
	}

	if (m_stateManager.comboMultiplier >= 3.0f)
	{
		m_hud.FlashCombo();
		if (m_consecutiveApples >= 5 && !m_comboSoundPlayed)
		{
			m_stateManager.GetAudio().PlaySound("combo_3x");
			m_comboSoundPlayed = true;
		}
	}
}

int PlayState::CalculatePoints(int l_base)
{
	return (int)(l_base * m_stateManager.comboMultiplier);
}

float PlayState::GetAppleTimerDuration() const
{
	// Level 10: timer controlled by phase system
	if (m_levelConfig.id == 10)
		return m_levelConfig.appleTimerSec;

	// Level 5 (Famine): progressive timer escalation
	float timer = m_levelConfig.appleTimerSec;
	if (m_applesEaten >= 15) timer = 2.0f;
	else if (m_applesEaten >= 10) timer = 3.0f;
	return timer;
}

void PlayState::RenderPhaseAnnouncement(Window& l_window)
{
	if (!m_announcementFontLoaded)
		return;

	float elapsed = m_announcementDuration - m_phaseAnnouncementTimer;

	// Fade in 0.3s, hold, fade out 0.5s
	float alpha = 1.0f;
	if (elapsed < 0.3f)
		alpha = elapsed / 0.3f;
	else if (m_phaseAnnouncementTimer < 0.5f)
		alpha = m_phaseAnnouncementTimer / 0.5f;

	sf::Uint8 a = (sf::Uint8)(255 * alpha);
	sf::Vector2u winSize = l_window.GetWindowSize();

	// Overlay opacity scales with text size (subtler for whispers)
	float overlayAlpha = (m_announcementCharSize >= 36) ? 120.0f : 60.0f;
	sf::RectangleShape overlay(sf::Vector2f((float)winSize.x, (float)winSize.y));
	overlay.setPosition(0.f, 0.f);
	overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(overlayAlpha * alpha)));
	l_window.Draw(overlay);

	// Announcement text, centered
	sf::Text text;
	text.setFont(m_announcementFont);
	text.setString(m_phaseAnnouncementText);
	text.setCharacterSize(m_announcementCharSize);

	// Whisper-sized announcements use lighter ink; phase announcements use bold red
	sf::Color textColor = (m_announcementCharSize >= 36)
		? sf::Color(180, 40, 30, a)
		: sf::Color(m_levelConfig.inkTint.r, m_levelConfig.inkTint.g,
					m_levelConfig.inkTint.b, a);
	text.setFillColor(textColor);

	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(bounds.left + bounds.width / 2.0f,
				   bounds.top + bounds.height / 2.0f);
	text.setPosition((float)winSize.x / 2.0f, (float)winSize.y / 2.0f);

	l_window.Draw(text);
}

void PlayState::CheckCruelMoment()
{
	// Don't overlap with existing announcements
	if (m_phaseAnnouncementTimer > 0.0f)
		return;

	const char* text = nullptr;
	int charSize = 22;

	int id = m_levelConfig.id;
	int a = m_applesEaten;
	int target = m_levelConfig.applesToWin;

	// Level-specific cruel moments
	if (id == 1 && a == target - 1)
		text = "One more. Easy, right?";
	else if (id == 2 && a == 6)
		text = "Getting used to the dark? Good.";
	else if (id == 3 && a == 10)
		text = "The floor is patient.";
	else if (id == 4 && a == 8)
		text = "Which one is real?";
	else if (id == 5 && a == target - 2)
		text = "Almost. If the timer agrees.";
	else if (id == 6 && a == 8)
		text = "Good luck.";
	else if (id == 7 && a == 5)
		text = "The ground doesn't like you.";
	else if (id == 8 && a == 3)
		text = "It's watching.";
	else if (id == 8 && a == 7)
		text = "It's learning.";
	else if (id == 9 && a == 5)
		text = "Remember the controls?";
	else if (id == 10 && a == 18)
	{
		text = "Two more. You can do this.";
		charSize = 28;
	}
	// Cross-level: first apple on 5+ retries
	else if (a == 1 && m_stateManager.deathCtx.retryCount >= 5)
		text = "Welcome back.";

	if (text)
	{
		m_phaseAnnouncementText = text;
		m_announcementDuration = 1.8f;
		m_phaseAnnouncementTimer = m_announcementDuration;
		m_announcementCharSize = charSize;
	}
}
