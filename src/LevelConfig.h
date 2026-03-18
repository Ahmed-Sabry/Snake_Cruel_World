#pragma once

#include "Platform/Platform.hpp"
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
};

inline std::array<LevelConfig, NUM_LEVELS> GetAllLevels()
{
	std::array<LevelConfig, NUM_LEVELS> levels;

	// Level 1: "False Hope"
	levels[0] = {
		1, "False Hope", "See? It's not so bad.",
		12.0f, 15, 4, 0.0f,
		sf::Color(30, 15, 20), sf::Color(200, 100, 50),
		sf::Color::Red, sf::Color::Magenta, sf::Color::Green,
		false, false, false, false, false, false, false, false,
		0.0f, 3, 0
	};

	// Level 2: "Lights Out"
	levels[1] = {
		2, "Lights Out", "Hope you memorized the layout.",
		11.0f, 12, 5, 0.0f,
		sf::Color(5, 5, 15), sf::Color(40, 40, 80),
		sf::Color(255, 60, 60), sf::Color(180, 40, 40), sf::Color(60, 255, 60),
		true, false, false, false, false, false, false, false,
		0.0f, 5, 2
	};

	// Level 3: "Quicksand"
	levels[2] = {
		3, "Quicksand", "Some tiles are hungrier than others.",
		13.0f, 15, 0, 8.0f,
		sf::Color(60, 45, 25), sf::Color(180, 120, 40),
		sf::Color::Red, sf::Color::Magenta, sf::Color::Green,
		false, true, false, false, false, false, false, false,
		0.0f, 10, 0
	};

	// Level 4: "Mirror Mirror"
	levels[3] = {
		4, "Mirror Mirror", "Trust nothing. Especially yourself.",
		11.0f, 12, 4, 0.0f,
		sf::Color(20, 20, 20), sf::Color(120, 120, 120),
		sf::Color::White, sf::Color(180, 180, 180), sf::Color::White,
		false, false, false, false, false, false, false, true,
		0.0f, 3, 1
	};

	// Level 5: "Famine"
	levels[4] = {
		5, "Famine", "Food is scarce. Patience is scarcer.",
		14.0f, 20, 5, 0.0f,
		sf::Color(25, 30, 20), sf::Color(80, 90, 50),
		sf::Color::Red, sf::Color::Magenta, sf::Color(255, 200, 0),
		false, false, false, false, false, false, true, false,
		5.0f, 3, 0
	};

	// Level 6: "Betrayal"
	levels[5] = {
		6, "Betrayal", "Not everything green is good for you.",
		12.0f, 12, 4, 0.0f,
		sf::Color(10, 30, 10), sf::Color(30, 80, 30),
		sf::Color::Red, sf::Color::Magenta, sf::Color::Green,
		false, false, false, false, false, true, false, false,
		0.0f, 2, 0
	};

	// Level 7: "Earthquake"
	levels[6] = {
		7, "Earthquake", "The ground has opinions about your route.",
		12.0f, 15, 3, 0.0f,
		sf::Color(35, 10, 5), sf::Color(160, 30, 20),
		sf::Color::Red, sf::Color::Magenta, sf::Color(255, 140, 0),
		false, false, false, true, false, false, false, false,
		0.0f, 5, 2
	};

	// Level 8: "The Watcher"
	levels[7] = {
		8, "The Watcher", "It learns.",
		13.0f, 10, 3, 0.0f,
		sf::Color(15, 15, 25), sf::Color(60, 70, 100),
		sf::Color::Red, sf::Color::Magenta, sf::Color::Green,
		false, false, true, false, false, false, false, false,
		0.0f, 5, 2
	};

	// Level 9: "Amnesia"
	levels[8] = {
		9, "Amnesia", "What controls? What controls.",
		10.0f, 15, 5, 0.0f,
		sf::Color(40, 10, 50), sf::Color::White,
		sf::Color::Red, sf::Color::Magenta, sf::Color(100, 200, 255),
		false, false, false, false, true, false, false, false,
		0.0f, 5, 2
	};

	// Level 10: "Cruel World"
	levels[9] = {
		10, "Cruel World", "Everything. All at once.",
		12.0f, 20, 4, 0.0f,
		sf::Color(30, 15, 20), sf::Color(200, 100, 50),
		sf::Color::Red, sf::Color::Magenta, sf::Color::Green,
		true, true, true, true, true, true, true, true,
		5.0f, 10, 5
	};

	return levels;
}
