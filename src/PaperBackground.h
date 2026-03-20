#pragma once

#include "Platform/Platform.hpp"
#include "LevelConfig.h"
#include "TextureGenerator.h"
#include "InkRenderer.h"

// Generates a per-level notebook paper background texture.
// Includes ruled lines, margin marks, age stains, and per-level margin doodles.
class PaperBackground
{
public:
	PaperBackground();
	~PaperBackground() = default;

	// Generate the background for a given level config
	void Generate(const LevelConfig& l_config, unsigned int l_windowWidth,
				  unsigned int l_windowHeight);

	// Draw the background to the target
	void Render(sf::RenderTarget& l_target);

	bool IsGenerated() const { return m_generated; }

private:
	void DrawRuledLines(sf::RenderTarget& l_target, const LevelConfig& l_config,
						unsigned int l_width, unsigned int l_height);
	void DrawMarginLine(sf::RenderTarget& l_target, const LevelConfig& l_config,
						unsigned int l_height);
	void DrawAgeStains(sf::RenderTarget& l_target, const LevelConfig& l_config,
					   unsigned int l_width, unsigned int l_height);
	void DrawMarginDoodle(sf::RenderTarget& l_target, const LevelConfig& l_config,
						  unsigned int l_width, unsigned int l_height);

	sf::RenderTexture m_renderTexture;
	sf::Sprite m_sprite;
	sf::Texture m_noiseTexture;
	sf::Font m_doodleFont;
	bool m_doodleFontLoaded;
	bool m_generated;
};
