#pragma once

#include "Platform/Platform.hpp"
#include <memory>

// Procedurally generates tileable textures at startup.
// All textures are white-on-transparent so ink tint can be applied via setColor().
class TextureGenerator
{
public:
	// Generate parallel line hatch pattern (tileable)
	// angle in degrees, spacing in pixels, lineWidth in pixels
	static sf::Texture GenerateHatchTexture(int l_size, float l_angleDeg,
											float l_spacing, float l_lineWidth);

	// Generate cross-hatch (two overlaid directions)
	static sf::Texture GenerateCrossHatchTexture(int l_size, float l_spacing,
												 float l_lineWidth);

	// Generate random dot stipple pattern
	static sf::Texture GenerateStippleTexture(int l_size, float l_density,
											  unsigned int l_seed = 42);

	// Generate simple noise texture (tileable-ish)
	static sf::Texture GenerateNoiseTexture(int l_width, int l_height,
											unsigned int l_seed = 123);

	// Simple hash for seeding
	static unsigned int Hash(unsigned int l_val);
};
