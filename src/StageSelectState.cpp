#include "StageSelectState.h"

#include "Ability.h"
#include "AudioManager.h"
#include "InkRenderer.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace
{
	sf::Text MakeText(const sf::Font& l_font, const std::string& l_string,
					  unsigned int l_size, const sf::Color& l_color,
					  float l_x, float l_y)
	{
		sf::Text text;
		text.setFont(l_font);
		text.setString(l_string);
		text.setCharacterSize(l_size);
		text.setFillColor(l_color);
		text.setPosition(l_x, l_y);
		return text;
	}
}

StageSelectState::StageSelectState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_animTimer(0.0f),
	  m_selectedIndex(0),
	  m_keyReleased(true)
{
}

void StageSelectState::OnEnter()
{
	if (!m_stateManager.HasUnlockedStageSelect())
	{
		m_stateManager.SwitchTo(StateType::MainMenu);
		return;
	}

	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "StageSelectState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	const sf::Vector2u winSize = window.GetWindowSize();

	LevelConfig bgConfig{};
	bgConfig.id = 0;
	bgConfig.paperTone = sf::Color(238, 228, 214);
	bgConfig.inkTint = sf::Color(55, 45, 40);
	bgConfig.corruption = 0.08f;
	m_paperBg.Generate(bgConfig, winSize.x, winSize.y);
	window.SetBackground(bgConfig.paperTone);

	m_title = MakeText(m_font, "Stage Select", 42, sf::Color(170, 55, 40), 0.0f, 26.0f);
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition(((float)winSize.x - titleBounds.width) / 2.0f, 26.0f);

	m_subtitle = MakeText(m_font, "Heal the notebook one page at a time.", 18,
		sf::Color(110, 90, 82), 0.0f, 78.0f);
	sf::FloatRect subtitleBounds = m_subtitle.getLocalBounds();
	m_subtitle.setPosition(((float)winSize.x - subtitleBounds.width) / 2.0f, 80.0f);

	m_footer = MakeText(m_font,
		"Enter - Play page   Esc - Main Menu   Endless unlocks at 4 healed pages",
		16, sf::Color(100, 90, 86), 28.0f, (float)winSize.y - 36.0f);

	m_statusStrip = MakeText(m_font, "", 18, sf::Color(70, 60, 54), 30.0f, 112.0f);

	BuildLayout();
	m_animTimer = 0.0f;
	m_keyReleased = false;

	m_selectedIndex = 0;
	if (m_stateManager.currentLevel == 10)
		m_selectedIndex = 8;
	else
	{
		bool found = false;
		for (std::size_t i = 0; i < m_tiles.size(); ++i)
		{
			if (m_tiles[i].levelId == m_stateManager.currentLevel)
			{
				m_selectedIndex = static_cast<int>(i);
				found = true;
				break;
			}
		}
		if (!found)
		{
			for (std::size_t i = 0; i < m_tiles.size(); ++i)
			{
				if (!m_stateManager.IsPageHealed(m_tiles[i].levelId))
				{
					m_selectedIndex = static_cast<int>(i);
					break;
				}
			}
		}
	}
}

void StageSelectState::OnExit()
{
}

void StageSelectState::BuildLayout()
{
	m_tiles.clear();

	const float startX = 70.0f;
	const float startY = 160.0f;
	const float tileWidth = 280.0f;
	const float tileHeight = 170.0f;
	const float gapX = 30.0f;
	const float gapY = 24.0f;

	std::vector<const LevelConfig*> pages;
	const auto& levels = GetAllLevels();
	for (const LevelConfig& config : levels)
	{
		if (config.id >= 2 && config.id <= 9)
			pages.push_back(&config);
	}

	std::sort(pages.begin(), pages.end(),
		[](const LevelConfig* l_lhs, const LevelConfig* l_rhs)
		{
			return l_lhs->stageSelectOrder < l_rhs->stageSelectOrder;
		});

	for (std::size_t i = 0; i < pages.size(); ++i)
	{
		const int col = static_cast<int>(i % 4);
		const int row = static_cast<int>(i / 4);
		StageTile tile;
		tile.levelId = pages[i]->id;
		tile.bounds = sf::FloatRect(
			startX + col * (tileWidth + gapX),
			startY + row * (tileHeight + gapY),
			tileWidth,
			tileHeight);
		m_tiles.push_back(tile);
	}

	m_finaleBounds = sf::FloatRect(463.0f, 532.0f, 440.0f, 120.0f);
}

int StageSelectState::GetSelectedLevelId() const
{
	if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tiles.size()))
		return m_tiles[static_cast<std::size_t>(m_selectedIndex)].levelId;
	return 10;
}

void StageSelectState::MoveSelectionHorizontal(int l_direction)
{
	if (m_selectedIndex >= 8)
		return;

	const int row = m_selectedIndex / 4;
	const int col = m_selectedIndex % 4;
	m_selectedIndex = row * 4 + (col + l_direction + 4) % 4;
}

void StageSelectState::MoveSelectionVertical(int l_direction)
{
	if (m_selectedIndex == 8)
	{
		if (l_direction < 0)
			m_selectedIndex = 6;
		return;
	}

	const int row = m_selectedIndex / 4;
	const int col = m_selectedIndex % 4;

	if (l_direction > 0)
	{
		if (row == 0)
			m_selectedIndex = 4 + col;
		else
			m_selectedIndex = 8;
	}
	else
	{
		if (row == 1)
			m_selectedIndex = col;
	}
}

void StageSelectState::ActivateSelection()
{
	const int levelId = GetSelectedLevelId();
	if (!m_stateManager.CanAccessCampaignLevel(levelId))
	{
		m_stateManager.GetAudio().PlaySound("menu_navigate");
		return;
	}

	m_stateManager.GetAudio().PlaySound("menu_select");
	m_stateManager.currentLevel = levelId;
	m_stateManager.SwitchTo(StateType::Gameplay);
}

sf::Color StageSelectState::GetTileOutlineColor(bool l_selected, bool l_available, bool l_healed) const
{
	if (!l_available)
		return sf::Color(120, 115, 112, 100);
	if (l_selected)
		return sf::Color(180, 55, 40, 180);
	if (l_healed)
		return sf::Color(50, 110, 70, 140);
	return sf::Color(80, 65, 58, 110);
}

void StageSelectState::DrawStageTile(Window& l_window, const StageTile& l_tile, bool l_selected)
{
	const LevelConfig& config = GetAllLevels()[static_cast<std::size_t>(l_tile.levelId - 1)];
	const bool healed = m_stateManager.IsPageHealed(l_tile.levelId);
	const bool available = m_stateManager.CanAccessCampaignLevel(l_tile.levelId);
	const StateManager::LevelProgress& progress = m_stateManager.GetLevelProgress(l_tile.levelId);
	sf::RenderTarget& target = l_window.GetRenderWindow();

	const sf::Color fill = healed ? sf::Color(225, 236, 224, 220) : sf::Color(235, 226, 215, 220);
	const sf::Color outline = GetTileOutlineColor(l_selected, available, healed);
	InkRenderer::DrawWobblyRect(target, l_tile.bounds.left, l_tile.bounds.top,
		l_tile.bounds.width, l_tile.bounds.height, fill, outline,
		l_selected ? 2.0f : 1.2f, healed ? 0.08f : 0.16f,
		static_cast<unsigned int>(l_tile.levelId * 37 + (l_selected ? 7 : 0)));

	const float x = l_tile.bounds.left + 14.0f;
	const float y = l_tile.bounds.top + 10.0f;
	const sf::Color textColor = available ? sf::Color(55, 45, 40) : sf::Color(130, 123, 118);
	const sf::Color accentColor = healed ? sf::Color(55, 110, 70) : sf::Color(150, 70, 48);

	sf::Text title = MakeText(m_font,
		std::to_string(config.id) + ". " + config.name, 24, textColor, x, y);
	sf::Text theme = MakeText(m_font,
		"Corruption: " + config.corruptionLabel, 16, accentColor, x, y + 34.0f);
	sf::Text hint = MakeText(m_font,
		"Hint: " + config.difficultyHint, 15, sf::Color(100, 88, 82), x, y + 60.0f);

	std::string stateLabel = healed ? "HEALED" : "CORRUPTED";
	sf::Text state = MakeText(m_font, stateLabel, 16,
		healed ? sf::Color(50, 110, 70) : sf::Color(150, 70, 48), x, y + 88.0f);

	std::ostringstream stats;
	stats << "Stars: " << progress.bestStars << "   Best: " << progress.bestScore;
	sf::Text statsText = MakeText(m_font, stats.str(), 15, sf::Color(100, 90, 84), x, y + 112.0f);

	std::string rewardLine = "Reward: ???";
	if (healed && config.abilityReward != AbilityId::None)
		rewardLine = "Reward: " + std::string(GetAbilityDefinition(config.abilityReward).name);
	sf::Text reward = MakeText(m_font, rewardLine, 15, textColor, x, y + 138.0f);

	l_window.Draw(title);
	l_window.Draw(theme);
	l_window.Draw(hint);
	l_window.Draw(state);
	l_window.Draw(statsText);
	l_window.Draw(reward);
}

void StageSelectState::DrawFinaleGate(Window& l_window, bool l_selected)
{
	const LevelConfig& finaleConfig = GetAllLevels()[9];
	const bool unlocked = m_stateManager.IsL10Unlocked();
	const bool completed = m_stateManager.HasCompletedLevel(10);
	const StateManager::LevelProgress& progress = m_stateManager.GetLevelProgress(10);
	sf::RenderTarget& target = l_window.GetRenderWindow();

	const sf::Color fill = unlocked ? sf::Color(232, 223, 214, 225) : sf::Color(220, 214, 210, 225);
	const sf::Color outline = GetTileOutlineColor(l_selected, unlocked || completed, completed);
	InkRenderer::DrawWobblyRect(target, m_finaleBounds.left, m_finaleBounds.top,
		m_finaleBounds.width, m_finaleBounds.height, fill, outline,
		l_selected ? 2.0f : 1.2f, unlocked ? 0.10f : 0.03f, 1009U);

	const float x = m_finaleBounds.left + 18.0f;
	const float y = m_finaleBounds.top + 12.0f;
	std::string gateState = completed ? "COMPLETED" : (unlocked ? "UNSEALED" : "SEALED");
	std::string gateHint = unlocked
		? "All healed pages converge here."
		: "Heal 8 / 8 pages to break the seal. Current: " + std::to_string(m_stateManager.GetHealedPageCount()) + "/8";

	l_window.Draw(MakeText(m_font, "Level 10: " + finaleConfig.name, 28,
		sf::Color(55, 45, 40), x, y));
	l_window.Draw(MakeText(m_font, "Gate: " + gateState, 17,
		completed ? sf::Color(50, 110, 70) : sf::Color(150, 70, 48), x, y + 38.0f));
	l_window.Draw(MakeText(m_font, "Threat: " + finaleConfig.corruptionLabel, 16,
		sf::Color(100, 88, 82), x, y + 66.0f));
	l_window.Draw(MakeText(m_font, gateHint, 16,
		sf::Color(90, 82, 78), x, y + 90.0f));

	std::ostringstream stats;
	stats << "Stars: " << progress.bestStars << "   Best: " << progress.bestScore;
	l_window.Draw(MakeText(m_font, stats.str(), 15,
		sf::Color(100, 90, 84), m_finaleBounds.left + 275.0f, y + 12.0f));
}

void StageSelectState::HandleInput()
{
	Window& window = m_stateManager.GetWindow();
	const bool upPressed = window.IsKeyPressed(sf::Keyboard::Up) || window.IsKeyPressed(sf::Keyboard::W);
	const bool downPressed = window.IsKeyPressed(sf::Keyboard::Down) || window.IsKeyPressed(sf::Keyboard::S);
	const bool leftPressed = window.IsKeyPressed(sf::Keyboard::Left) || window.IsKeyPressed(sf::Keyboard::A);
	const bool rightPressed = window.IsKeyPressed(sf::Keyboard::Right) || window.IsKeyPressed(sf::Keyboard::D);
	const bool enterPressed = window.IsKeyPressed(sf::Keyboard::Return) || window.IsKeyPressed(sf::Keyboard::Space);
	const bool escPressed = window.IsKeyPressed(sf::Keyboard::Escape);

	if (!upPressed && !downPressed && !leftPressed && !rightPressed && !enterPressed && !escPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;

	if (escPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::MainMenu);
		return;
	}

	if (leftPressed)
	{
		MoveSelectionHorizontal(-1);
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (rightPressed)
	{
		MoveSelectionHorizontal(1);
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (upPressed)
	{
		MoveSelectionVertical(-1);
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (downPressed)
	{
		MoveSelectionVertical(1);
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (enterPressed)
	{
		ActivateSelection();
	}
}

void StageSelectState::Update(float l_dt)
{
	m_animTimer += l_dt;

	int unlockedAbilityCount = 0;
	for (bool unlocked : m_stateManager.unlockedAbilities)
	{
		if (unlocked)
			++unlockedAbilityCount;
	}

	const AbilityDefinition& equipped = GetAbilityDefinition(m_stateManager.equippedAbility);
	std::ostringstream status;
	status << "Healed Pages: " << m_stateManager.GetHealedPageCount() << "/8"
		   << "   Unlocked Abilities: " << unlockedAbilityCount
		   << "   Equipped: " << equipped.name;
	m_statusStrip.setString(status.str());
}

void StageSelectState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	window.Draw(m_title);
	window.Draw(m_subtitle);
	window.Draw(m_footer);
	window.Draw(m_statusStrip);

	sf::FloatRect titleBounds = m_title.getGlobalBounds();
	InkRenderer::DrawWobblyLine(target,
		titleBounds.left, titleBounds.top + titleBounds.height + 6.0f,
		titleBounds.left + titleBounds.width, titleBounds.top + titleBounds.height + 6.0f,
		sf::Color(170, 55, 40, 120), 1.8f, 0.12f, 2701U);

	for (std::size_t i = 0; i < m_tiles.size(); ++i)
		DrawStageTile(window, m_tiles[i], static_cast<int>(i) == m_selectedIndex);

	DrawFinaleGate(window, m_selectedIndex == 8);
}
