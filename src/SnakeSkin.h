#pragma once

#include "Window.h"
#include <vector>

enum class SkinRenderFlag : int
{
	None         = 0,
	Translucent  = 1 << 0,
	Rainbow      = 1 << 1,
	Gradient     = 1 << 2,
	ThickOutline = 1 << 3,
	InvertEyes   = 1 << 4,
};

inline int operator|(SkinRenderFlag a, SkinRenderFlag b)
{
	return static_cast<int>(a) | static_cast<int>(b);
}

struct SnakeSkin
{
	int id;
	const char* name;
	const char* description;
	int unlockLevel;        // 3-star this level to unlock (0 = default/always)
	sf::Color headColor;
	sf::Color bodyColor;
	sf::Color gradientEnd;  // used if Gradient flag set
	int renderFlags;        // bitmask of SkinRenderFlag
};

inline std::vector<SnakeSkin> GetAllSkins()
{
	return {
		// 0: Classic — uses level's native colors, no overrides
		{ 0, "Classic", "Default snake colors",
		  0, sf::Color::Transparent, sf::Color::Transparent,
		  sf::Color::Transparent, 0 },

		// 1: Ghost — translucent pale blue
		{ 1, "Ghost", "3-star Level 1",
		  1, sf::Color(220, 220, 240), sf::Color(180, 180, 200),
		  sf::Color::Transparent, static_cast<int>(SkinRenderFlag::Translucent) },

		// 2: Midnight — dark blue with thick outline
		{ 2, "Midnight", "3-star Level 2",
		  2, sf::Color(30, 30, 80), sf::Color(15, 15, 50),
		  sf::Color::Transparent, static_cast<int>(SkinRenderFlag::ThickOutline) },

		// 3: Sandy — gold/brown gradient
		{ 3, "Sandy", "3-star Level 3",
		  3, sf::Color(210, 180, 100), sf::Color(180, 150, 80),
		  sf::Color(120, 90, 40), static_cast<int>(SkinRenderFlag::Gradient) },

		// 4: Mirror — silver with inverted eyes
		{ 4, "Mirror", "3-star Level 4",
		  4, sf::Color(200, 200, 210), sf::Color(160, 160, 170),
		  sf::Color::Transparent, static_cast<int>(SkinRenderFlag::InvertEyes) },

		// 5: Starving — bone white
		{ 5, "Starving", "3-star Level 5",
		  5, sf::Color(240, 235, 220), sf::Color(200, 195, 180),
		  sf::Color::Transparent, 0 },

		// 6: Toxic — neon green with thick outline
		{ 6, "Toxic", "3-star Level 6",
		  6, sf::Color(0, 255, 80), sf::Color(0, 200, 50),
		  sf::Color::Transparent, static_cast<int>(SkinRenderFlag::ThickOutline) },

		// 7: Volcanic — orange/red gradient
		{ 7, "Volcanic", "3-star Level 7",
		  7, sf::Color(255, 100, 20), sf::Color(200, 50, 10),
		  sf::Color(80, 10, 0), static_cast<int>(SkinRenderFlag::Gradient) },

		// 8: Hunter — blue/cyan
		{ 8, "Hunter", "3-star Level 8",
		  8, sf::Color(40, 180, 220), sf::Color(20, 120, 180),
		  sf::Color::Transparent, 0 },

		// 9: Glitch — rainbow cycling
		{ 9, "Glitch", "3-star Level 9",
		  9, sf::Color::White, sf::Color::White,
		  sf::Color::Transparent, static_cast<int>(SkinRenderFlag::Rainbow) },

		// 10: Cruel — near-black with thick red outline
		{ 10, "Cruel", "3-star Level 10",
		  10, sf::Color(20, 5, 5), sf::Color(10, 0, 0),
		  sf::Color::Transparent, static_cast<int>(SkinRenderFlag::ThickOutline) },
	};
}
