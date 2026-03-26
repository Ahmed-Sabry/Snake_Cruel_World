#pragma once

#include "GameState.h"
#include "Ability.h"
#include "AbilityHUD.h"
#include "StateManager.h"
#include "Snake.h"
#include "World.h"
#include "HUD.h"
#include "LevelConfig.h"
#include "ParticleSystem.h"
#include "ScreenShake.h"
#include "BlackoutEffect.h"
#include "Quicksand.h"
#include "MirrorGhost.h"
#include "TimedApple.h"
#include "PoisonApple.h"
#include "Earthquake.h"
#include "Predator.h"
#include "ControlShuffle.h"
#include "PaperBackground.h"
#include "PostProcessor.h"
#include "AchievementNotification.h"
#include "EndlessModeController.h"
#include <memory>

class PlayState : public BaseState
{
public:
	PlayState(StateManager& l_stateManager);
	~PlayState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

private:
	enum class EncounterPhase
	{
		Stage,
		BossTransition,
		BossCombat,
		BossReward
	};

	void OnAppleEaten(const Position& l_applePos);
	void OnDeath();
	void UpdateCombo(bool l_reset);
	int CalculatePoints(int l_base);
	int CalculateStars() const;
	float GetAppleTimerDuration() const;
	void InitCruelWorldPhases();
	void AdvanceCruelPhase();
	void RenderPhaseAnnouncement(Window& l_window);
	void CheckCruelMoment();
	void SyncAbilityState();
	bool LevelUsesBossEncounter() const;
	BossContext BuildBossContext() const;
	void BeginBossEncounter();
	void UpdateBossEncounter(float l_dt);
	void ApplyBossProgressEvent(const BossProgressEvent& l_event);
	void CompleteEncounterVictory(bool l_healPage, const std::string& l_cutsceneId,
								  bool l_bossEncounterSkipped = false);

	Snake m_snake;
	World m_world;
	HUD m_hud;
	AbilityHUD m_abilityHud;
	LevelConfig m_levelConfig;
	ParticleSystem m_particles;
	ScreenShake m_screenShake;

	// Level mechanic systems
	BlackoutEffect m_blackout;
	Quicksand m_quicksand;
	MirrorGhost m_mirrorGhost;
	TimedApple m_timedApple;
	PoisonApple m_poisonApple;
	Earthquake m_earthquake;
	Predator m_predator;
	int m_predatorApplesEaten;
	ControlShuffle m_controlShuffle;
	sf::CircleShape m_realPulse; // reused each frame for Phase 2 apple pulse
	int m_mirrorFlipCounter;

	float m_psychedelicTimer;

	float m_elapsedTime;
	float m_gameTime;
	int m_applesEaten;
	int m_consecutiveApples; // for combo
	float m_speedModifier;

	int m_lastShrinkCount;
	bool m_cheatExtend; // temp cheat code
	bool m_escReleased;
	bool m_rReleased;
	bool m_cycleReleased;
	bool m_activateReleased;
	bool m_comboSoundPlayed;
	float m_levelCompleteDelay; // countdown before switching to GameOver
	AbilityController m_abilityController;
	EncounterPhase m_encounterPhase;
	std::unique_ptr<Boss> m_activeBoss;

	// Announcement / "Cruel World" phase system
	int m_cruelPhase;
	bool m_screenFlipped;
	float m_phaseAnnouncementTimer;
	float m_announcementDuration;     // total duration for current announcement
	int m_announcementCharSize;       // character size (48 for phases, 22 for whispers)
	std::string m_phaseAnnouncementText;
	sf::Font m_announcementFont;
	bool m_announcementFontLoaded;

	// "Living Ink" visual systems
	PaperBackground m_paperBackground;
	PostProcessor m_postProcessor;
	bool m_postProcessorInited;

	// Transition animations
	float m_pageTurnTimer;       // Page-turn entry animation (slides bg from right)
	float m_pageTurnDuration;
	float m_deathInkRunTimer;    // Ink-run death effect (ink drips downward)
	float m_deathInkRunDuration;
	bool m_deathInkRunActive;

	// Apple burst outline effect
	float m_appleBurstTimer;
	sf::Vector2f m_appleBurstPos;
	sf::Color m_appleBurstColor;

	// Border hatch fill animation on shrink
	float m_borderHatchTimer;
	float m_borderHatchDuration;

	// Achievement/statistics tracking
	int m_quicksandTouches;
	bool m_wasOnQuicksand;
	int m_timedAppleMisses;
	int m_poisonApplesThisLevel;
	bool m_reachedMinBody;            // self-collision reduced body to 1 segment
	float m_screenFlipStartTime;      // timestamp when Level 10 screen flipped

	// Achievement notification popup
	AchievementNotification m_achievementNotif;

	// Audio polish
	float m_heartbeatTimer;

	// Control shuffle grace period tracking
	bool m_wasInGracePeriod;

	// Endless mode
	std::unique_ptr<EndlessModeController> m_endlessCtrl;
	float m_endlessWarningTimer;
	std::string m_endlessWarningText;
};
