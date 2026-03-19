#pragma once

#include "GameState.h"
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
	void OnAppleEaten(const Position& l_applePos);
	void OnDeath();
	void UpdateCombo(bool l_reset);
	int CalculatePoints(int l_base);
	float GetAppleTimerDuration() const;

	Snake m_snake;
	World m_world;
	HUD m_hud;
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
	sf::CircleShape m_realPulse; // reused each frame for Phase 2 apple pulse
	int m_mirrorFlipCounter;

	float m_elapsedTime;
	float m_gameTime;
	int m_applesEaten;
	int m_consecutiveApples; // for combo
	float m_speedModifier;

	int m_lastShrinkCount;
	bool m_cheatExtend; // temp cheat code
	bool m_escReleased;
	bool m_rReleased;
	bool m_comboSoundPlayed;
	float m_levelCompleteDelay; // countdown before switching to GameOver
};
