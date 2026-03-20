#include "CutsceneState.h"
#include "CutsceneDefs.h"
#include "LevelConfig.h"
#include "AudioManager.h"
#include <iostream>

CutsceneState* CutsceneState::s_active = nullptr;

CutsceneState::CutsceneState(StateManager& l_stateManager)
	: BaseState(l_stateManager)
{
}

void CutsceneState::OnEnter()
{
	s_active = this;

	m_fontLoaded = m_font.loadFromFile(FONT_PATH);
	if (!m_fontLoaded)
		std::cerr << "CutsceneState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();

	// Initialize post-processor
	m_postProcessor.Init(winSize.x, winSize.y);
	LevelConfig defaultConfig{};
	defaultConfig.id = 99;
	defaultConfig.paperTone = sf::Color(245, 235, 220);
	defaultConfig.inkTint = sf::Color(60, 50, 45);
	defaultConfig.corruption = 0.02f;
	defaultConfig.enableInkBleed = false;
	defaultConfig.enableChromatic = false;
	defaultConfig.enablePsychedelic = false;
	m_postProcessor.Configure(defaultConfig);

	// Generate default paper background
	m_paperBg.Generate(defaultConfig, winSize.x, winSize.y);
	window.SetBackground(defaultConfig.paperTone);

	// Clear scene
	m_scene.Clear();
	m_particles.Clear();

	// Build timeline from cutscene ID
	m_timeline = CutsceneDefs::Build(m_stateManager.cutsceneId, m_stateManager);
	m_timeline.Start(m_stateManager);

	m_inputPressed = false;
	m_skipRequested = false;
}

void CutsceneState::OnExit()
{
	s_active = nullptr;
	m_screenShake.Reset(m_stateManager.GetWindow());
	m_scene.Clear();
	m_particles.Clear();
}

void CutsceneState::HandleInput()
{
	bool escPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);

	if (escPressed && !m_inputPressed)
	{
		m_skipRequested = true;
	}

	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) ||
						sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	if (enterPressed && !m_inputPressed)
	{
		m_timeline.SkipCurrent();
	}

	m_inputPressed = escPressed || enterPressed;
	if (!escPressed && !enterPressed)
		m_inputPressed = false;
}

void CutsceneState::Update(float l_dt)
{
	// Skip entire cutscene
	if (m_skipRequested)
	{
		if (m_stateManager.cutsceneOnSkip)
			m_stateManager.cutsceneOnSkip(m_stateManager);
		m_stateManager.SwitchTo(m_stateManager.cutsceneReturnState);
		return;
	}

	m_screenShake.Update(l_dt, m_stateManager.GetWindow());
	m_particles.Update(l_dt);
	m_postProcessor.Update(l_dt);

	bool done = m_timeline.Update(l_dt, m_stateManager);
	if (done)
		m_stateManager.SwitchTo(m_stateManager.cutsceneReturnState);
}

void CutsceneState::Render()
{
	Window& window = m_stateManager.GetWindow();
	bool ppAvailable = m_postProcessor.IsAvailable();

	if (ppAvailable)
		m_postProcessor.Begin();

	sf::RenderTarget& target = ppAvailable
		? m_postProcessor.GetTarget()
		: (sf::RenderTarget&)window.GetRenderWindow();

	// Paper background
	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	// Scene entities
	m_scene.Render(target, m_font);

	// Timeline (persistent text, current action renders)
	m_timeline.Render(target);

	// Particles
	m_particles.RenderTo(target);

	if (ppAvailable)
	{
		m_postProcessor.End();
		m_postProcessor.Apply(window);
	}
}
