#pragma once

#include "Platform/Platform.hpp"
#include "Ability.h"
#include <string>
#include <array>

static constexpr int NUM_LEVELS = 10;
inline const std::string FONT_PATH = "Fonts/RujisHandwritingFontV20-vrqZ.ttf";

struct LevelConfig
{
	int id;
	std::string name;
	std::string subtitle;
	float baseSpeed;
	int applesToWin;
	int shrinkInterval; // 0 = time-based only
	float shrinkTimerSec; // 0 = apple-based only
	sf::Color background;
	sf::Color border;
	sf::Color snakeHead;
	sf::Color snakeBody;
	sf::Color apple;
	AbilityId abilityReward;
	bool hasBlackouts;
	bool hasQuicksand;
	bool hasPredator;
	bool hasEarthquakes;
	bool hasControlShuffle;
	bool hasPoisonApples;
	bool hasTimedApples;
	bool hasMirrorGhost;
	float appleTimerSec; // for timed apples (Famine)
	int starThreshold2; // max self-collisions for 2 stars
	int starThreshold3; // max self-collisions for 3 stars

	// "Living Ink" visual parameters
	sf::Color paperTone;    // Background paper color
	sf::Color inkTint;      // Primary ink color for drawing
	sf::Color accentColor;  // Accent/highlight color
	float corruption;       // 0.0 (pristine) to 1.0 (chaotic) - controls wobble, stains, etc.
	bool enableInkBleed;    // Enable ink bleed/wobble post-processing shader
	bool enableChromatic;   // Enable chromatic aberration shader
	bool enablePsychedelic; // Enable psychedelic hue shift shader
};

inline const std::array<LevelConfig, NUM_LEVELS>& GetAllLevels()
{
	static const std::array<LevelConfig, NUM_LEVELS> levels = []()
	{
		std::array<LevelConfig, NUM_LEVELS> l;

		// Level 1: "False Hope" -- Fresh blue-black ballpoint pen on new notebook
	l[0] = {
		1, "False Hope", "See? It's not so bad.",
		12.0f, 15, 4, 0.0f,
		sf::Color(28, 22, 30), sf::Color(175, 120, 75),
		sf::Color(55, 45, 65), sf::Color(80, 70, 95), sf::Color(180, 55, 45), AbilityId::None,
		false, false, false, false, false, false, false, false,
		0.0f, 3, 0,
		sf::Color(248, 242, 228), sf::Color(45, 40, 55), sf::Color(170, 65, 55),
		0.05f, false, false, false
	};

	// Level 2: "Lights Out" -- Dark India ink with amber candlelight
	l[1] = {
		2, "Lights Out", "Hope you memorized the layout.",
		11.0f, 12, 5, 0.0f,
		sf::Color(8, 8, 18), sf::Color(45, 45, 75),
		sf::Color(210, 120, 50), sf::Color(170, 95, 40), sf::Color(230, 175, 55), AbilityId::InkFlare,
		true, false, false, false, false, false, false, false,
		0.0f, 5, 2,
		sf::Color(195, 185, 165), sf::Color(25, 25, 55), sf::Color(220, 130, 65),
		0.15f, false, false, false
	};

	// Level 3: "Quicksand" -- Sepia fountain pen on sun-bleached journal
	l[2] = {
		3, "Quicksand", "Some tiles are hungrier than others.",
		13.0f, 15, 0, 8.0f,
		sf::Color(55, 42, 25), sf::Color(170, 115, 45),
		sf::Color(90, 50, 25), sf::Color(130, 80, 40), sf::Color(165, 40, 35), AbilityId::ShedSkin,
		false, true, false, false, false, false, false, false,
		0.0f, 10, 0,
		sf::Color(215, 195, 155), sf::Color(110, 65, 30), sf::Color(190, 145, 45),
		0.25f, false, false, false
	};

	// Level 4: "Mirror Mirror" -- Graphite pencil on white drafting paper
	l[3] = {
		4, "Mirror Mirror", "Trust nothing. Especially yourself.",
		11.0f, 12, 4, 0.0f,
		sf::Color(18, 18, 22), sf::Color(115, 115, 120),
		sf::Color(35, 35, 40), sf::Color(70, 70, 75), sf::Color(230, 230, 225), AbilityId::ShadowDecoy,
		false, false, false, false, false, false, false, true,
		0.0f, 3, 1,
		sf::Color(242, 242, 240), sf::Color(18, 18, 20), sf::Color(175, 175, 180),
		0.30f, false, false, false
	};

	// Level 5: "Famine" -- Fading olive-green ink on dried parchment
	l[4] = {
		5, "Famine", "Food is scarce. Patience is scarcer.",
		14.0f, 20, 5, 0.0f,
		sf::Color(25, 28, 18), sf::Color(85, 85, 45),
		sf::Color(65, 75, 30), sf::Color(90, 100, 50), sf::Color(210, 170, 40), AbilityId::TimeFreeze,
		false, false, false, false, false, false, true, false,
		5.0f, 3, 0,
		sf::Color(225, 205, 170), sf::Color(55, 65, 35), sf::Color(195, 170, 50),
		0.40f, false, false, false
	};

	// Level 6: "Betrayal" -- Forest-green ink on poisoned page
	l[5] = {
		6, "Betrayal", "Not everything green is good for you.",
		12.0f, 12, 4, 0.0f,
		sf::Color(12, 28, 12), sf::Color(35, 75, 35),
		sf::Color(55, 100, 50), sf::Color(75, 120, 65), sf::Color(185, 150, 40), AbilityId::VenomTrail,
		false, false, false, false, false, true, false, false,
		0.0f, 2, 0,
		sf::Color(205, 212, 195), sf::Color(25, 50, 25), sf::Color(75, 160, 55),
		0.50f, false, false, false
	};

	// Level 7: "Earthquake" -- Rust-red ink on terracotta paper
	l[6] = {
		7, "Earthquake", "The ground has opinions about your route.",
		12.0f, 15, 3, 0.0f,
		sf::Color(32, 12, 8), sf::Color(150, 40, 25),
		sf::Color(140, 45, 20), sf::Color(170, 70, 35), sf::Color(215, 180, 50), AbilityId::InkAnchor,
		false, false, false, true, false, false, false, false,
		0.0f, 5, 2,
		sf::Color(205, 175, 155), sf::Color(130, 35, 20), sf::Color(220, 120, 30),
		0.60f, true, false, false
	};

	// Level 8: "The Watcher" -- Cold steel-blue ink on clinical gray paper
	l[7] = {
		8, "The Watcher", "It learns.",
		13.0f, 10, 3, 0.0f,
		sf::Color(14, 16, 26), sf::Color(55, 65, 95),
		sf::Color(50, 55, 90), sf::Color(70, 75, 110), sf::Color(200, 80, 60), AbilityId::HuntersDash,
		false, false, true, false, false, false, false, false,
		0.0f, 5, 2,
		sf::Color(212, 218, 228), sf::Color(40, 45, 65), sf::Color(220, 225, 235),
		0.70f, true, false, false
	};

	// Level 9: "Amnesia" -- Purple-violet ink bleeding on wet paper
	l[8] = {
		9, "Amnesia", "What controls? What controls.",
		10.0f, 15, 5, 0.0f,
		sf::Color(38, 12, 48), sf::Color(210, 200, 230),
		sf::Color(80, 35, 100), sf::Color(110, 55, 130), sf::Color(100, 180, 210), AbilityId::InkMemory,
		false, false, false, false, true, false, false, false,
		0.0f, 5, 2,
		sf::Color(218, 205, 228), sf::Color(50, 25, 60), sf::Color(160, 90, 200),
		0.85f, true, true, true
	};

	// Level 10: "Cruel World" -- 4-phase escalation, starts as L1 callback
	l[9] = {
		10, "Cruel World", "Everything. All at once.",
		12.0f, 20, 4, 0.0f,
		sf::Color(28, 22, 30), sf::Color(175, 120, 75),
		sf::Color(55, 45, 65), sf::Color(80, 70, 95), sf::Color(180, 55, 45), AbilityId::None,
		true, true, true, true, true, true, true, true,
		6.0f, 10, 5,
		sf::Color(248, 242, 228), sf::Color(45, 40, 55), sf::Color(170, 65, 55),
		1.00f, true, true, true
	};

		return l;
	}();
	return levels;
}
