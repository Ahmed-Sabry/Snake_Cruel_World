#include "ControlShuffle.h"
#include "RandomUtils.h"
#include <algorithm>
#include <cmath>
#include <string>

ControlShuffle::ControlShuffle()
	: m_state(ShuffleState::Idle),
	  m_timer(0.0f),
	  m_indicatorTimer(0.0f),
	  m_graceTimer(0.0f),
	  m_applesEaten(0),
	  m_justShuffled(false),
	  m_justEnteredWarning(false),
	  m_fontLoaded(false)
{
	m_mapping = { Direction::Up, Direction::Down, Direction::Right, Direction::Left };
	m_shownIndices[0] = 0;
	m_shownIndices[1] = 1;
}

void ControlShuffle::Reset()
{
	m_state = ShuffleState::Idle;
	m_timer = 0.0f;
	m_indicatorTimer = 0.0f;
	m_graceTimer = 0.0f;
	m_applesEaten = 0;
	m_justShuffled = false;
	m_justEnteredWarning = false;
	m_mapping = { Direction::Up, Direction::Down, Direction::Right, Direction::Left };
	m_shownIndices[0] = 0;
	m_shownIndices[1] = 1;

	if (!m_fontLoaded)
	{
		m_font.loadFromFile(FONT_PATH);
		m_indicatorText.setFont(m_font);
		m_indicatorText.setCharacterSize(18);
		m_indicatorText.setFillColor(sf::Color::White);
		m_indicatorBg.setFillColor(sf::Color(40, 10, 50, 180));
		m_fontLoaded = true;
	}
}

// --- Phase system ---

int ControlShuffle::GetPhase() const
{
	if (m_applesEaten < 5)  return 0; // Learning
	if (m_applesEaten < 10) return 1; // Chaos
	return 2;                          // Amnesia
}

float ControlShuffle::GetIdleDuration() const
{
	float intervals[] = { 15.0f, 12.0f, 9.0f };
	return intervals[GetPhase()] - GetWarningDuration();
}

float ControlShuffle::GetWarningDuration() const
{
	float durations[] = { 2.0f, 2.0f, 1.0f };
	return durations[GetPhase()];
}

float ControlShuffle::GetIndicatorDuration() const
{
	float durations[] = { 2.5f, 1.5f, 1.0f };
	return durations[GetPhase()];
}

int ControlShuffle::GetIndicatorCount() const
{
	return GetPhase() < 2 ? 4 : 2;
}

// --- State machine ---

void ControlShuffle::Update(float l_dt)
{
	m_timer += l_dt;

	switch (m_state)
	{
		case ShuffleState::Idle:
		{
			if (m_timer >= GetIdleDuration())
			{
				m_state = ShuffleState::Warning;
				m_timer = 0.0f;
				m_justEnteredWarning = true;
			}
			break;
		}

		case ShuffleState::Warning:
		{
			if (m_timer >= GetWarningDuration())
			{
				Shuffle();
				m_graceTimer = 0.5f;
				m_indicatorTimer = GetIndicatorDuration();
				m_justShuffled = true;
				m_state = ShuffleState::Indicating;
				m_timer = 0.0f;
			}
			break;
		}

		case ShuffleState::Indicating:
		{
			m_indicatorTimer -= l_dt;
			if (m_indicatorTimer < 0.0f)
				m_indicatorTimer = 0.0f;

			if (m_indicatorTimer <= 0.0f)
			{
				m_state = ShuffleState::Idle;
				m_timer = 0.0f;
			}
			break;
		}

		default:
			break;
	}

	// Grace timer (independent of state)
	if (m_graceTimer > 0.0f)
	{
		m_graceTimer -= l_dt;
		if (m_graceTimer < 0.0f)
			m_graceTimer = 0.0f;
	}
}

void ControlShuffle::Shuffle()
{
	std::array<Direction, 4> prev = m_mapping;

	if (GetPhase() == 0)
	{
		// Learning phase: only swap one axis
		m_mapping = { Direction::Up, Direction::Down, Direction::Right, Direction::Left }; // identity
		int axis = RandomInt(0, 1);
		if (axis == 0)
		{
			// Swap Up/Down
			m_mapping[0] = Direction::Down;
			m_mapping[1] = Direction::Up;
		}
		else
		{
			// Swap Left/Right
			m_mapping[2] = Direction::Left;
			m_mapping[3] = Direction::Right;
		}

		// Ensure different from previous
		if (m_mapping == prev)
		{
			// Swap the other axis instead
			m_mapping = { Direction::Up, Direction::Down, Direction::Right, Direction::Left };
			if (axis == 0)
			{
				m_mapping[2] = Direction::Left;
				m_mapping[3] = Direction::Right;
			}
			else
			{
				m_mapping[0] = Direction::Down;
				m_mapping[1] = Direction::Up;
			}
		}
	}
	else
	{
		// Chaos / Amnesia: full Fisher-Yates shuffle
		std::array<Direction, 4> dirs = { Direction::Up, Direction::Down, Direction::Right, Direction::Left };
		for (int i = 3; i > 0; i--)
		{
			int j = RandomInt(0, i);
			std::swap(dirs[i], dirs[j]);
		}

		if (dirs == prev)
			std::swap(dirs[0], dirs[1]);

		m_mapping = dirs;
	}

	// Pick which 2 indices to show in partial indicator (phase 2)
	if (GetIndicatorCount() == 2)
	{
		m_shownIndices[0] = RandomInt(0, 3);
		do {
			m_shownIndices[1] = RandomInt(0, 3);
		} while (m_shownIndices[1] == m_shownIndices[0]);
	}
}

Direction ControlShuffle::MapDirection(Direction l_input) const
{
	if (l_input == Direction::None)
		return Direction::None;
	return m_mapping[(int)l_input - 1];
}

void ControlShuffle::OnAppleEaten(int l_totalApples)
{
	m_applesEaten = l_totalApples;
}

bool ControlShuffle::JustShuffled()
{
	if (m_justShuffled)
	{
		m_justShuffled = false;
		return true;
	}
	return false;
}

bool ControlShuffle::IsGracePeriod() const
{
	return m_graceTimer > 0.0f;
}

bool ControlShuffle::IsWarning()
{
	if (m_justEnteredWarning)
	{
		m_justEnteredWarning = false;
		return true;
	}
	return false;
}

const char* ControlShuffle::DirectionLabel(Direction l_dir) const
{
	switch (l_dir)
	{
		case Direction::Up:    return "Up";
		case Direction::Down:  return "Down";
		case Direction::Left:  return "Left";
		case Direction::Right: return "Right";
		default:               return "?";
	}
}

void ControlShuffle::Render(Window& l_window)
{
	sf::Vector2u winSize = l_window.GetWindowSize();
	float barMaxW = 200.0f;
	float barH = 6.0f;
	float bottomY = (float)winSize.y - 30.0f;

	// --- Warning state: countdown bar ---
	if (m_state == ShuffleState::Warning)
	{
		float progress = m_timer / GetWarningDuration(); // 0→1
		float barW = barMaxW * (1.0f - progress);        // depletes left-to-right

		// Color: white → yellow → red
		sf::Uint8 r = 255;
		sf::Uint8 g, b;
		if (progress < 0.5f)
		{
			g = 255;
			b = (sf::Uint8)(255 * (1.0f - progress * 2.0f)); // 255→0
		}
		else
		{
			g = (sf::Uint8)(255 * (1.0f - (progress - 0.5f) * 2.0f)); // 255→0
			b = 0;
		}

		// Background bar (dark outline)
		float bgX = ((float)winSize.x - barMaxW) / 2.0f;
		m_warningBarBg.setSize(sf::Vector2f(barMaxW, barH));
		m_warningBarBg.setPosition(bgX, bottomY);
		m_warningBarBg.setFillColor(sf::Color(60, 60, 60, 180));
		l_window.Draw(m_warningBarBg);

		// Filling bar
		m_warningBar.setSize(sf::Vector2f(barW, barH));
		m_warningBar.setPosition(bgX, bottomY);
		m_warningBar.setFillColor(sf::Color(r, g, b, 230));
		l_window.Draw(m_warningBar);

		return;
	}

	// --- Indicating state: mapping text ---
	if (m_state == ShuffleState::Indicating && m_indicatorTimer > 0.0f)
	{
		// Fade-out alpha in the last 0.5 seconds
		float alpha = std::min(1.0f, m_indicatorTimer / 0.5f);
		sf::Uint8 a = (sf::Uint8)(255 * alpha);

		const char* keys[] = { "W", "S", "A", "D" };
		Direction inputs[] = { Direction::Up, Direction::Down, Direction::Left, Direction::Right };
		int count = GetIndicatorCount();

		std::string text;

		if (count == 4)
		{
			// Show all 4 mappings
			for (int i = 0; i < 4; i++)
			{
				if (i > 0)
					text += "    ";
				text += keys[i];
				text += ": ";
				text += DirectionLabel(m_mapping[(int)inputs[i] - 1]);
			}
		}
		else
		{
			// Partial indicator: show only 2, mark others as "??"
			for (int i = 0; i < 4; i++)
			{
				if (i > 0)
					text += "    ";

				if (i == m_shownIndices[0] || i == m_shownIndices[1])
				{
					text += keys[i];
					text += ": ";
					text += DirectionLabel(m_mapping[(int)inputs[i] - 1]);
				}
				else
				{
					text += keys[i];
					text += ": ??";
				}
			}
		}

		m_indicatorText.setString(text);
		m_indicatorText.setFillColor(sf::Color(255, 255, 255, a));

		// Position at bottom-center
		sf::FloatRect textBounds = m_indicatorText.getLocalBounds();
		float padding = 12.0f;
		float bgW = textBounds.width + padding * 2;
		float bgH = textBounds.height + padding * 2;
		float bgX = ((float)winSize.x - bgW) / 2.0f;
		float bgY = (float)winSize.y - bgH - 20.0f;

		m_indicatorBg.setSize(sf::Vector2f(bgW, bgH));
		m_indicatorBg.setPosition(bgX, bgY);
		m_indicatorBg.setFillColor(sf::Color(40, 10, 50, (sf::Uint8)(180 * alpha)));

		m_indicatorText.setPosition(bgX + padding - textBounds.left, bgY + padding - textBounds.top);

		l_window.Draw(m_indicatorBg);
		l_window.Draw(m_indicatorText);
	}
}
