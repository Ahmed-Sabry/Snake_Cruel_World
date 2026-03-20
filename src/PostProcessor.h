#pragma once

#include "Platform/Platform.hpp"
#include "Window.h"
#include "LevelConfig.h"

// Post-processing pipeline using ping-pong RenderTextures and GLSL shaders.
// Applies paper overlay, vignette, ink bleed, chromatic aberration, and psychedelic effects.
// Falls back gracefully if shaders unavailable.
class PostProcessor
{
public:
	PostProcessor();
	~PostProcessor() = default;

	// Initialize with window dimensions. Must be called before use.
	bool Init(unsigned int l_width, unsigned int l_height);

	// Configure for a specific level's visual parameters
	void Configure(const LevelConfig& l_config);

	// Begin rendering: redirect draw calls to the offscreen RT
	void Begin();

	// End rendering: stop redirecting
	void End();

	// Apply shader chain and draw result to the given window
	void Apply(Window& l_window);

	// Get the render target to draw to (between Begin/End)
	sf::RenderTarget& GetTarget();

	// Whether shaders are available and post-processing is active
	bool IsAvailable() const { return m_available; }

	// Update time-dependent shader uniforms
	void Update(float l_dt);

	// Set blackout parameters (Level 2)
	void SetBlackoutParams(bool l_active, float l_headX, float l_headY);

private:
	bool LoadShaders();

	sf::RenderTexture m_rtA;
	sf::RenderTexture m_rtB;
	sf::Sprite m_spriteA;
	sf::Sprite m_spriteB;

	sf::Shader m_paperShader;
	sf::Shader m_vignetteShader;
	sf::Shader m_inkBleedShader;
	sf::Shader m_chromaticShader;
	sf::Shader m_psychedelicShader;

	sf::Texture m_paperNoiseTex;

	bool m_available;
	bool m_initialized;
	bool m_rendering;

	// Current config
	float m_vignetteStrength;
	float m_vignetteRadius;
	bool m_enableInkBleed;
	float m_inkBleedAmount;
	bool m_enableChromatic;
	float m_chromaticAmount;
	bool m_enablePsychedelic;
	float m_psychedelicIntensity;

	float m_time;
	sf::Vector2f m_resolution;
};
