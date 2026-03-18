#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "Snake.h"
#include "World.h"
#include "HUD.h"
#include "LevelConfig.h"
#include "ParticleSystem.h"
#include "ScreenShake.h"

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
	void OnAppleEaten();
	void OnDeath();
	void UpdateCombo(bool l_reset);
	int CalculatePoints(int l_base);

	Snake m_snake;
	World m_world;
	HUD m_hud;
	LevelConfig m_levelConfig;
	ParticleSystem m_particles;
	ScreenShake m_screenShake;

	float m_elapsedTime;
	float m_gameTime;
	int m_applesEaten;
	int m_consecutiveApples; // for combo

	int m_lastShrinkCount;
	bool m_cheatExtend; // temp cheat code
	bool m_escReleased;
	bool m_rReleased;
	bool m_comboSoundPlayed;
};
