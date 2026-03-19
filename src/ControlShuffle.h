#pragma once

#include "Window.h"
#include "Snake.h"
#include "LevelConfig.h"
#include <array>

enum class ShuffleState
{
	Idle,
	Warning,
	Indicating
};

class ControlShuffle
{
public:
	ControlShuffle();

	void Reset();
	void Update(float l_dt);
	void Render(Window& l_window);

	Direction MapDirection(Direction l_input) const;
	void OnAppleEaten(int l_totalApples);
	bool JustShuffled();
	bool IsGracePeriod() const;
	bool IsWarning();

private:
	void Shuffle();
	const char* DirectionLabel(Direction l_dir) const;
	int GetPhase() const;
	float GetIdleDuration() const;
	float GetWarningDuration() const;
	float GetIndicatorDuration() const;
	int GetIndicatorCount() const;

	ShuffleState m_state;
	float m_timer;
	float m_indicatorTimer;
	float m_graceTimer;

	std::array<Direction, 4> m_mapping; // indexed by (int)dir - 1
	int m_applesEaten;
	bool m_justShuffled;
	bool m_justEnteredWarning;
	bool m_fontLoaded;

	// Partial indicator: which 2 indices to show in phase 2
	int m_shownIndices[2];

	sf::Font m_font;
	sf::Text m_indicatorText;
	sf::RectangleShape m_indicatorBg;
	sf::RectangleShape m_warningBarBg;
	sf::RectangleShape m_warningBar;
};
