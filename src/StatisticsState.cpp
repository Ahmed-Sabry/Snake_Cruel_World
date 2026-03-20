#include "StatisticsState.h"
#include "StatsManager.h"
#include "AudioManager.h"
#include "LevelConfig.h"
#include "InkRenderer.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

static std::string FormatTime(int l_totalSeconds)
{
	int h = l_totalSeconds / 3600;
	int m = (l_totalSeconds % 3600) / 60;
	int s = l_totalSeconds % 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << h << ":"
		<< std::setfill('0') << std::setw(2) << m << ":"
		<< std::setfill('0') << std::setw(2) << s;
	return oss.str();
}

static std::string FormatLevelTime(float l_seconds)
{
	if (l_seconds <= 0.0f) return "--:--";
	int m = (int)l_seconds / 60;
	int s = (int)l_seconds % 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << m << ":"
		<< std::setfill('0') << std::setw(2) << s;
	return oss.str();
}

StatisticsState::StatisticsState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_keyReleased(true),
	  m_scrollOffset(0),
	  m_maxVisibleLines(0),
	  m_lineSpacing(28.f)
{
}

void StatisticsState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "StatisticsState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();

	// Paper background
	LevelConfig menuConfig{};
	menuConfig.id = 0;
	menuConfig.paperTone = sf::Color(245, 235, 220);
	menuConfig.inkTint = sf::Color(60, 50, 45);
	menuConfig.corruption = 0.02f;
	m_paperBg.Generate(menuConfig, winSize.x, winSize.y);
	window.SetBackground(menuConfig.paperTone);

	// Title
	m_title.setFont(m_font);
	m_title.setString("~ Your Cruel Journey ~");
	m_title.setCharacterSize(40);
	m_title.setFillColor(sf::Color(180, 50, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 40.f);

	// Build stat lines
	m_lines.clear();
	const CumulativeStats& stats = m_stateManager.GetStats().GetStats();
	sf::Color inkColor(60, 50, 45);
	sf::Color dimColor(100, 90, 85);
	float lineX = 120.f;
	float lineY = 110.f;
	float lineSpacing = m_lineSpacing;

	auto addLine = [&](const std::string& text, sf::Color color = sf::Color(60, 50, 45),
					   unsigned int size = 20)
	{
		sf::Text t;
		t.setFont(m_font);
		t.setString(text);
		t.setCharacterSize(size);
		t.setFillColor(color);
		t.setPosition(lineX, lineY);
		lineY += lineSpacing;
		m_lines.push_back(std::move(t));
	};

	auto addCentered = [&](const std::string& text, sf::Color color = sf::Color(60, 50, 45),
						   unsigned int size = 22)
	{
		sf::Text t;
		t.setFont(m_font);
		t.setString(text);
		t.setCharacterSize(size);
		t.setFillColor(color);
		sf::FloatRect b = t.getLocalBounds();
		t.setPosition((winSize.x - b.width) / 2.0f, lineY);
		lineY += lineSpacing + 4.f;
		m_lines.push_back(std::move(t));
	};

	// Global stats
	addLine("Total Playtime:          " + FormatTime(stats.totalPlaytimeSeconds));
	addLine("Total Apples Eaten:      " + std::to_string(stats.totalApplesEaten));
	addLine("Total Deaths:            " + std::to_string(m_stateManager.totalDeaths));
	addLine("Segments Lost:           " + std::to_string(stats.totalSegmentsLost));
	addLine("Poison Apples Eaten:     " + std::to_string(stats.totalPoisonApplesEaten));
	addLine("Best Combo:              " + std::to_string(stats.bestCombo) + "x");
	addLine("Best Single Score:       " + std::to_string(stats.bestSingleLevelScore));
	addLine("Predator Apples Stolen:  " + std::to_string(stats.predatorApplesStolen));
	addLine("Predator Deaths:         " + std::to_string(stats.predatorKills));

	lineY += 10.f;
	addCentered("--- Per Level ---", sf::Color(180, 50, 40), 22);

	auto levels = GetAllLevels();
	for (int i = 0; i < NUM_LEVELS; i++)
	{
		bool unlocked = (i + 1) <= m_stateManager.highestUnlockedLevel;
		if (!unlocked)
		{
			addLine("Level " + std::to_string(i + 1) + "  ???", dimColor, 18);
			continue;
		}

		int attempts = stats.attemptsPerLevel[i];
		int deaths = stats.deathsPerLevel[i];
		int clears = std::max(0, attempts - deaths);
		std::string best = FormatLevelTime(stats.fastestLevelTimes[i]);

		std::ostringstream oss;
		oss << "Level " << (i + 1) << " \"" << levels[i].name << "\"  "
			<< attempts << " attempts | " << clears << " clears | Best: " << best
			<< " | Deaths: " << deaths;
		addLine(oss.str(), inkColor, 17);
	}

	// Endless mode stats
	if (stats.endlessBestScore > 0 || stats.endlessBestTime > 0.0f)
	{
		lineY += 10.f;
		addCentered("--- Endless Mode ---", sf::Color(180, 50, 40), 22);
		addLine("Best Score: " + std::to_string(stats.endlessBestScore) +
				"      Best Time: " + FormatLevelTime(stats.endlessBestTime));
	}

	lineY += 20.f;
	addCentered("[ESC] Back", dimColor, 18);

	m_scrollOffset = 0;
	m_maxVisibleLines = std::max(1, (int)((winSize.y - 120.f) / lineSpacing));
	m_keyReleased = false;
}

void StatisticsState::OnExit()
{
}

void StatisticsState::HandleInput()
{
	bool escPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);

	if (!escPressed && !upPressed && !downPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased) return;
	m_keyReleased = false;

	if (escPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::MainMenu);
	}
	else if (upPressed && m_scrollOffset > 0)
	{
		m_scrollOffset--;
	}
	else if (downPressed)
	{
		int maxScroll = std::max(0, (int)m_lines.size() - m_maxVisibleLines);
		if (m_scrollOffset < maxScroll)
			m_scrollOffset++;
	}
}

void StatisticsState::Update(float l_dt)
{
	(void)l_dt;
}

void StatisticsState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	window.Draw(m_title);

	// Draw lines with scroll offset
	float scrollPixels = m_scrollOffset * m_lineSpacing;
	for (size_t i = 0; i < m_lines.size(); i++)
	{
		sf::Text& line = m_lines[i];
		float y = line.getPosition().y - scrollPixels;
		if (y < 100.f || y > (float)window.GetWindowSize().y - 20.f)
			continue;
		sf::Vector2f origPos = line.getPosition();
		line.setPosition(origPos.x, y);
		window.Draw(line);
		line.setPosition(origPos); // restore
	}
}
