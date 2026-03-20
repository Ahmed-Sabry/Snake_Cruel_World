#include "AchievementState.h"
#include "AchievementManager.h"
#include "AchievementDefs.h"
#include "AudioManager.h"
#include "LevelConfig.h"
#include "InkRenderer.h"
#include <iostream>

AchievementState::AchievementState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_keyReleased(true),
	  m_scrollOffset(0),
	  m_maxVisibleLines(0)
{
}

void AchievementState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "AchievementState: Failed to load font" << std::endl;

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

	AchievementManager& mgr = m_stateManager.GetAchievements();
	auto allAchievements = GetAllAchievements();
	int unlocked = mgr.GetUnlockedCount();
	int total = NUM_ACHIEVEMENTS;

	// Title
	m_title.setFont(m_font);
	m_title.setString("~ Achievements ~");
	m_title.setCharacterSize(40);
	m_title.setFillColor(sf::Color(180, 50, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 35.f);

	// Counter
	m_counter.setFont(m_font);
	m_counter.setString("[" + std::to_string(unlocked) + "/" + std::to_string(total) + "]");
	m_counter.setCharacterSize(22);
	m_counter.setFillColor(sf::Color(100, 90, 85));
	sf::FloatRect counterBounds = m_counter.getLocalBounds();
	m_counter.setPosition((winSize.x - counterBounds.width) / 2.0f, 80.f);

	// Build achievement lines grouped by category
	m_lines.clear();
	sf::Color inkColor(60, 50, 45);
	sf::Color dimColor(130, 120, 115);
	sf::Color goldColor(180, 140, 40);
	sf::Color sectionColor(180, 50, 40);
	float lineX = 100.f;
	float lineY = 115.f;
	float lineSpacing = 28.f;

	auto addSectionHeader = [&](const std::string& text)
	{
		sf::Text t;
		t.setFont(m_font);
		t.setString("=== " + text + " ===");
		t.setCharacterSize(22);
		t.setFillColor(sectionColor);
		sf::FloatRect b = t.getLocalBounds();
		t.setPosition((winSize.x - b.width) / 2.0f, lineY);
		lineY += lineSpacing + 4.f;
		m_lines.push_back(std::move(t));
	};

	auto addAchievement = [&](const AchievementDef& def)
	{
		bool isUnlocked = mgr.IsUnlocked(def.id);
		sf::Text t;
		t.setFont(m_font);

		std::string checkbox = isUnlocked ? "[*] " : "[ ] ";
		std::string name = (def.hidden && !isUnlocked) ? "???" : def.name;
		std::string desc = (def.hidden && !isUnlocked) ? "???" : def.description;

		t.setString(checkbox + name + "    " + desc);
		t.setCharacterSize(18);
		t.setFillColor(isUnlocked ? goldColor : dimColor);
		t.setPosition(lineX, lineY);
		lineY += lineSpacing;
		m_lines.push_back(std::move(t));
	};

	// Group by category
	AchievementCategory categories[] = {
		AchievementCategory::Skill,
		AchievementCategory::Cruelty,
		AchievementCategory::Persistence,
		AchievementCategory::Hidden
	};
	const char* categoryNames[] = { "Skill", "Cruelty", "Persistence", "Hidden" };

	for (int c = 0; c < 4; c++)
	{
		addSectionHeader(categoryNames[c]);
		for (const auto& def : allAchievements)
		{
			if (def.category == categories[c])
				addAchievement(def);
		}
		lineY += 6.f; // spacing between categories
	}

	lineY += 10.f;
	{
		sf::Text t;
		t.setFont(m_font);
		t.setString("[ESC] Back");
		t.setCharacterSize(18);
		t.setFillColor(dimColor);
		sf::FloatRect b = t.getLocalBounds();
		t.setPosition((winSize.x - b.width) / 2.0f, lineY);
		m_lines.push_back(std::move(t));
	}

	m_scrollOffset = 0;
	m_maxVisibleLines = (int)((winSize.y - 120.f) / lineSpacing);
	m_keyReleased = false;
}

void AchievementState::OnExit()
{
}

void AchievementState::HandleInput()
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

void AchievementState::Update(float l_dt)
{
	(void)l_dt;
}

void AchievementState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	window.Draw(m_title);
	window.Draw(m_counter);

	// Draw lines with scroll offset
	float scrollPixels = m_scrollOffset * 28.f;
	for (size_t i = 0; i < m_lines.size(); i++)
	{
		sf::Text& line = m_lines[i];
		float y = line.getPosition().y - scrollPixels;
		if (y < 108.f || y > (float)window.GetWindowSize().y - 20.f)
			continue;
		sf::Vector2f origPos = line.getPosition();
		line.setPosition(origPos.x, y);
		window.Draw(line);
		line.setPosition(origPos);
	}
}
