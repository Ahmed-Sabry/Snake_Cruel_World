#pragma once

#include "GameState.h"
#include "StateManager.h"
#include "CutsceneTimeline.h"
#include "CutsceneScene.h"
#include "CutsceneCamera.h"
#include "PaperBackground.h"
#include "ParticleSystem.h"
#include "PostProcessor.h"
#include "ScreenShake.h"

class CutsceneState : public BaseState
{
public:
	CutsceneState(StateManager& l_stateManager);
	~CutsceneState() = default;

	void OnEnter() override;
	void OnExit() override;
	void HandleInput() override;
	void Update(float l_dt) override;
	void Render() override;

	CutsceneScene& GetScene() { return m_scene; }
	CutsceneCamera& GetCamera() { return m_camera; }
	ParticleSystem& GetParticles() { return m_particles; }
	PostProcessor& GetPostProcessor() { return m_postProcessor; }
	ScreenShake& GetScreenShake() { return m_screenShake; }
	PaperBackground& GetPaperBg() { return m_paperBg; }
	sf::Font& GetFont() { return m_font; }
	void ClearAllPersistent() { m_timeline.ClearAllPersistent(); }

	void CaptureFrame(sf::RenderTexture& l_target);

	// Set in OnEnter(), cleared in OnExit(). Non-owning. Only valid during the active cutscene state.
	static CutsceneState* s_active;

private:
	CutsceneTimeline m_timeline;
	CutsceneScene m_scene;
	CutsceneCamera m_camera;
	PaperBackground m_paperBg;
	ParticleSystem m_particles;
	PostProcessor m_postProcessor;
	ScreenShake m_screenShake;
	sf::Font m_font;
	bool m_fontLoaded = false;
	bool m_inputPressed = false;
	bool m_skipRequested = false;
};
