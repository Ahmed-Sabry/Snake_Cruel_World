#include "TextureGenerator.h"
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

unsigned int TextureGenerator::Hash(unsigned int l_val)
{
	l_val ^= l_val >> 16;
	l_val *= 0x45d9f3bu;
	l_val ^= l_val >> 16;
	l_val *= 0x45d9f3bu;
	l_val ^= l_val >> 16;
	return l_val;
}

sf::Texture TextureGenerator::GenerateHatchTexture(int l_size, float l_angleDeg,
												   float l_spacing, float l_lineWidth)
{
	if (l_size <= 0 || l_spacing <= 0.0f)
		return sf::Texture();

	sf::Image img;
	img.create(l_size, l_size, sf::Color::Transparent);

	float angleRad = l_angleDeg * (float)M_PI / 180.0f;
	float cosA = std::cos(angleRad);
	float sinA = std::sin(angleRad);
	float halfLine = l_lineWidth * 0.5f;

	for (int y = 0; y < l_size; y++)
	{
		for (int x = 0; x < l_size; x++)
		{
			// Distance from pixel to nearest hatch line
			// Project onto perpendicular direction
			float perpDist = -sinA * (float)x + cosA * (float)y;
			perpDist = std::fmod(std::fmod(perpDist, l_spacing) + l_spacing, l_spacing);

			// Modulo by spacing for tiling
			float modDist = perpDist;
			if (modDist > l_spacing * 0.5f)
				modDist = l_spacing - modDist;

			if (modDist <= halfLine)
			{
				// Anti-alias at edges
				float alpha = 1.0f;
				if (modDist > halfLine - 0.5f)
					alpha = (halfLine - modDist) / 0.5f;
				img.setPixel(x, y, sf::Color(255, 255, 255, (sf::Uint8)(255 * alpha)));
			}
		}
	}

	sf::Texture tex;
	tex.loadFromImage(img);
	tex.setRepeated(true);
	tex.setSmooth(false);
	return tex;
}

sf::Texture TextureGenerator::GenerateCrossHatchTexture(int l_size, float l_spacing,
														float l_lineWidth)
{
	if (l_size <= 0 || l_spacing <= 0.0f)
		return sf::Texture();

	sf::Image img;
	img.create(l_size, l_size, sf::Color::Transparent);

	auto addHatch = [&](float angleDeg)
	{
		float angleRad = angleDeg * (float)M_PI / 180.0f;
		float cosA = std::cos(angleRad);
		float sinA = std::sin(angleRad);
		float halfLine = l_lineWidth * 0.5f;

		for (int y = 0; y < l_size; y++)
		{
			for (int x = 0; x < l_size; x++)
			{
				float perpDist = -sinA * (float)x + cosA * (float)y;
				perpDist = std::fmod(std::fmod(perpDist, l_spacing) + l_spacing, l_spacing);
				float modDist = perpDist;
				if (modDist > l_spacing * 0.5f)
					modDist = l_spacing - modDist;

				if (modDist <= halfLine)
				{
					sf::Color existing = img.getPixel(x, y);
					sf::Uint8 newA = std::min(255, (int)existing.a + 180);
					img.setPixel(x, y, sf::Color(255, 255, 255, newA));
				}
			}
		}
	};

	addHatch(45.0f);
	addHatch(135.0f);

	sf::Texture tex;
	tex.loadFromImage(img);
	tex.setRepeated(true);
	tex.setSmooth(false);
	return tex;
}

sf::Texture TextureGenerator::GenerateStippleTexture(int l_size, float l_density,
													 unsigned int l_seed)
{
	if (l_size <= 0)
		return sf::Texture();

	sf::Image img;
	img.create(l_size, l_size, sf::Color::Transparent);

	int totalPixels = l_size * l_size;
	int dotCount = (int)(totalPixels * l_density);

	for (int i = 0; i < dotCount; i++)
	{
		unsigned int h = Hash(l_seed + (unsigned int)i);
		int x = (int)(h % l_size);
		int y = (int)((h >> 8) % l_size);
		sf::Uint8 alpha = (sf::Uint8)(150 + (h >> 16) % 106);
		img.setPixel(x, y, sf::Color(255, 255, 255, alpha));
	}

	sf::Texture tex;
	tex.loadFromImage(img);
	tex.setRepeated(true);
	tex.setSmooth(false);
	return tex;
}

sf::Texture TextureGenerator::GenerateNoiseTexture(int l_width, int l_height,
												   unsigned int l_seed)
{
	if (l_width <= 0 || l_height <= 0)
		return sf::Texture();

	sf::Image img;
	img.create(l_width, l_height, sf::Color(128, 128, 128, 255));

	for (int y = 0; y < l_height; y++)
	{
		for (int x = 0; x < l_width; x++)
		{
			unsigned int h = Hash(l_seed + (unsigned int)(y * l_width + x));
			// Low-frequency noise: blend with neighbors conceptually
			// Simple value noise approach
			sf::Uint8 val = (sf::Uint8)(100 + (h & 0xFF) % 56);
			img.setPixel(x, y, sf::Color(val, val, val, 30));
		}
	}

	sf::Texture tex;
	tex.loadFromImage(img);
	tex.setRepeated(true);
	tex.setSmooth(true);
	return tex;
}
