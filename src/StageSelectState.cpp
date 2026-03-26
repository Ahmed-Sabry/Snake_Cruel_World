#include "StageSelectState.h"

#include "StateManager.h"
#include "Ability.h"
#include "AudioManager.h"
#include "InkRenderer.h"
#include <algorithm>
#include <cmath>
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

	void CenterText(sf::Text& l_text, float l_x, float l_y)
	{
		const sf::FloatRect bounds = l_text.getLocalBounds();
		l_text.setOrigin(bounds.left + bounds.width / 2.0f,
			bounds.top + bounds.height / 2.0f);
		l_text.setPosition(l_x, l_y);
	}
}

StageSelectState::StageSelectState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_animTimer(0.0f),
	  m_selectedIndex(0),
	  m_lastGridSelection(5),
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
	bgConfig.paperTone = sf::Color(240, 232, 218);
	bgConfig.inkTint = sf::Color(50, 40, 35);
	bgConfig.corruption = 0.05f;
	m_paperBg.Generate(bgConfig, winSize.x, winSize.y);
	window.SetBackground(bgConfig.paperTone);

	m_title = MakeText(m_font, "Stage Select", 40, sf::Color(180, 50, 40), 0.0f, 0.0f);
	CenterText(m_title, winSize.x / 2.0f, 46.0f);

	m_subtitle = MakeText(m_font, "Heal the notebook one page at a time.", 18,
		sf::Color(110, 90, 82), 0.0f, 0.0f);
	CenterText(m_subtitle, winSize.x / 2.0f, 82.0f);

	m_footer = MakeText(m_font,
		"Arrows / WASD - Move   Enter - Play page   Esc - Main Menu",
		16, sf::Color(100, 90, 86), 0.0f, 0.0f);
	CenterText(m_footer, winSize.x / 2.0f, (float)winSize.y - 28.0f);

	m_statusStrip = MakeText(m_font, "", 16, sf::Color(88, 76, 68), 0.0f, 0.0f);

	BuildLayout();
	m_animTimer = 0.0f;
	m_keyReleased = false;

	m_selectedIndex = 0;
	if (m_stateManager.currentLevel == 10)
	{
		m_selectedIndex = 8;
		m_lastGridSelection = 5;
	}
	else
	{
		bool found = false;
		for (std::size_t i = 0; i < m_tiles.size(); ++i)
		{
			if (m_tiles[i].levelId == m_stateManager.currentLevel)
			{
				m_selectedIndex = static_cast<int>(i);
				m_lastGridSelection = m_selectedIndex;
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
					m_lastGridSelection = m_selectedIndex;
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

	const sf::Vector2u winSize = m_stateManager.GetWindow().GetWindowSize();
	const float sideMargin = 62.0f;
	const float startY = 156.0f;
	const float gapX = 24.0f;
	const float gapY = 22.0f;
	const float gridWidth = (float)winSize.x - sideMargin * 2.0f;
	const float tileWidth = (gridWidth - gapX * 3.0f) / 4.0f;
	const float tileHeight = 130.0f;

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
			sideMargin + col * (tileWidth + gapX),
			startY + row * (tileHeight + gapY),
			tileWidth,
			tileHeight);
		m_tiles.push_back(tile);
	}

	const float gridBottom = startY + tileHeight * 2.0f + gapY;
	const float finaleWidth = tileWidth * 2.0f + gapX;
	m_finaleBounds = sf::FloatRect(
		sideMargin + (gridWidth - finaleWidth) / 2.0f,
		gridBottom + 28.0f,
		finaleWidth,
		108.0f);
	m_detailBounds = sf::FloatRect(
		sideMargin,
		m_finaleBounds.top + m_finaleBounds.height + 18.0f,
		gridWidth,
		96.0f);
}

int StageSelectState::GetSelectedLevelId() const
{
	if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_tiles.size()))
		return m_tiles[static_cast<std::size_t>(m_selectedIndex)].levelId;
	return 10;
}

void StageSelectState::MoveSelectionHorizontal(int l_direction)
{
	if (m_selectedIndex == 8)
	{
		m_selectedIndex = (l_direction < 0) ? 5 : 6;
		m_lastGridSelection = m_selectedIndex;
		return;
	}

	const int row = m_selectedIndex / 4;
	const int col = m_selectedIndex % 4;
	m_selectedIndex = row * 4 + (col + l_direction + 4) % 4;
	m_lastGridSelection = m_selectedIndex;
}

void StageSelectState::MoveSelectionVertical(int l_direction)
{
	if (m_selectedIndex == 8)
	{
		if (l_direction < 0)
		{
			if (m_lastGridSelection < 4 || m_lastGridSelection > 7)
				m_lastGridSelection = 5;
			m_selectedIndex = m_lastGridSelection;
		}
		return;
	}

	const int row = m_selectedIndex / 4;
	const int col = m_selectedIndex % 4;

	if (l_direction > 0)
	{
		if (row == 0)
		{
			m_selectedIndex = 4 + col;
			m_lastGridSelection = m_selectedIndex;
		}
		else if (col == 1 || col == 2)
		{
			m_lastGridSelection = m_selectedIndex;
			m_selectedIndex = 8;
		}
	}
	else
	{
		if (row == 1)
		{
			m_selectedIndex = col;
			m_lastGridSelection = m_selectedIndex;
		}
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

sf::Color StageSelectState::GetSelectionPulseColor(bool l_available) const
{
	const float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
	if (!l_available)
	{
		return sf::Color(150, 126, 112,
			static_cast<sf::Uint8>(96 + pulse * 40.0f));
	}

	return sf::Color(static_cast<sf::Uint8>(150 + pulse * 40.0f),
		48, 38, static_cast<sf::Uint8>(120 + pulse * 70.0f));
}

sf::Color StageSelectState::GetTileOutlineColor(bool l_selected, bool l_available, bool l_healed) const
{
	if (!l_available)
		return l_selected ? GetSelectionPulseColor(false) : sf::Color(138, 132, 126, 105);
	if (l_selected)
		return GetSelectionPulseColor(true);
	if (l_healed)
		return sf::Color(72, 118, 84, 130);
	return sf::Color(86, 70, 62, 112);
}

void StageSelectState::DrawStars(sf::RenderTarget& l_target, float l_x, float l_y,
	int l_stars, const sf::Color& l_outlineColor, unsigned int l_seed) const
{
	for (int star = 0; star < 3; ++star)
	{
		const bool filled = star < l_stars;
		InkRenderer::DrawStar(l_target, l_x + star * 18.0f, l_y, 6.5f,
			filled ? sf::Color(205, 175, 50) : sf::Color::Transparent,
			l_outlineColor, 0.12f, filled, l_seed + static_cast<unsigned int>(star * 17));
	}
}

void StageSelectState::DrawStageTile(Window& l_window, const StageTile& l_tile, bool l_selected)
{
	const LevelConfig& config = GetAllLevels()[static_cast<std::size_t>(l_tile.levelId - 1)];
	const bool healed = m_stateManager.IsPageHealed(l_tile.levelId);
	const bool available = m_stateManager.CanAccessCampaignLevel(l_tile.levelId);
	const StateManager::LevelProgress& progress = m_stateManager.GetLevelProgress(l_tile.levelId);
	sf::RenderTarget& target = l_window.GetRenderWindow();

	const sf::Color fill = !available
		? sf::Color(229, 223, 216, 220)
		: (healed ? sf::Color(229, 236, 225, 224) : sf::Color(236, 228, 218, 224));
	const sf::Color outline = GetTileOutlineColor(l_selected, available, healed);

	if (l_selected)
	{
		InkRenderer::DrawWobblyRect(target, l_tile.bounds.left - 4.0f, l_tile.bounds.top - 4.0f,
			l_tile.bounds.width + 8.0f, l_tile.bounds.height + 8.0f,
			sf::Color::Transparent, GetSelectionPulseColor(available), 1.6f,
			0.24f, static_cast<unsigned int>(m_animTimer * 10.0f) + l_tile.levelId * 19U);
	}

	InkRenderer::DrawWobblyRect(target, l_tile.bounds.left, l_tile.bounds.top,
		l_tile.bounds.width, l_tile.bounds.height, fill, outline,
		l_selected ? 1.5f : 1.0f, healed ? 0.06f : 0.08f,
		static_cast<unsigned int>(l_tile.levelId * 37 + (l_selected ? 7 : 0)));

	const float x = l_tile.bounds.left + 18.0f;
	const float y = l_tile.bounds.top + 12.0f;
	const sf::Color titleColor = l_selected
		? sf::Color(GetSelectionPulseColor(available).r, 42, 34)
		: (available ? sf::Color(55, 45, 40) : sf::Color(130, 123, 118));
	const sf::Color subtitleColor = available ? sf::Color(118, 100, 92) : sf::Color(146, 138, 132);
	const sf::Color accentColor = !available
		? sf::Color(136, 130, 126)
		: (healed ? sf::Color(58, 110, 72) : sf::Color(145, 74, 52));

	sf::Text title = MakeText(m_font,
		std::to_string(config.id) + ". " + config.name, 23, titleColor, x, y);
	sf::Text subtitle = MakeText(m_font, config.subtitle, 14, subtitleColor, x, y + 30.0f);

	const std::string stateLabel = !available
		? "LOCKED"
		: (healed ? "PAGE HEALED"
			: (config.bossConfig.enabled && progress.stageCleared
				? "BOSS AWAITS"
				: "PAGE CORRUPTED"));
	sf::Text state = MakeText(m_font, stateLabel, 15, accentColor, x, y + 60.0f);

	std::ostringstream bestStream;
	bestStream << "Best: " << progress.bestScore;
	sf::Text bestText = MakeText(m_font, bestStream.str(), 15,
		sf::Color(100, 90, 84), x + 82.0f, y + 84.0f);

	l_window.Draw(title);
	l_window.Draw(subtitle);
	l_window.Draw(state);
	DrawStars(target, x + 8.0f, y + 96.0f, progress.bestStars,
		sf::Color(78, 68, 60, 150), static_cast<unsigned int>(config.id * 101));
	l_window.Draw(bestText);
}

void StageSelectState::DrawFinaleGate(Window& l_window, bool l_selected)
{
	const LevelConfig& finaleConfig = GetAllLevels()[9];
	const bool unlocked = m_stateManager.IsL10Unlocked();
	const bool completed = m_stateManager.HasCompletedLevel(10);
	const StateManager::LevelProgress& progress = m_stateManager.GetLevelProgress(10);
	sf::RenderTarget& target = l_window.GetRenderWindow();

	const bool available = unlocked || completed;
	const sf::Color fill = available ? sf::Color(236, 228, 220, 225) : sf::Color(226, 220, 215, 225);
	const sf::Color outline = GetTileOutlineColor(l_selected, available, completed);

	if (l_selected)
	{
		InkRenderer::DrawWobblyRect(target, m_finaleBounds.left - 4.0f, m_finaleBounds.top - 4.0f,
			m_finaleBounds.width + 8.0f, m_finaleBounds.height + 8.0f,
			sf::Color::Transparent, GetSelectionPulseColor(available), 1.6f,
			0.24f, static_cast<unsigned int>(m_animTimer * 12.0f) + 1009U);
	}

	InkRenderer::DrawWobblyRect(target, m_finaleBounds.left, m_finaleBounds.top,
		m_finaleBounds.width, m_finaleBounds.height, fill, outline,
		l_selected ? 1.5f : 1.0f, available ? 0.07f : 0.04f, 1009U);

	const float x = m_finaleBounds.left + 20.0f;
	const float y = m_finaleBounds.top + 14.0f;
	const std::string gateState = completed ? "Gate Open" : (unlocked ? "Gate Unsealed" : "Gate Sealed");
	const sf::Color titleColor = l_selected
		? sf::Color(GetSelectionPulseColor(available).r, 42, 34)
		: (available ? sf::Color(55, 45, 40) : sf::Color(130, 123, 118));

	sf::Text titleLine = MakeText(m_font, "10. " + finaleConfig.name, 28, titleColor, x, y);
	sf::Text subtitleLine = MakeText(m_font, finaleConfig.subtitle, 15,
		sf::Color(118, 100, 92), x, y + 34.0f);
	sf::Text gateLine = MakeText(m_font, gateState, 18,
		completed ? sf::Color(58, 110, 72) : sf::Color(145, 74, 52), x, y + 64.0f);

	std::ostringstream stats;
	stats << "Best: " << progress.bestScore;
	sf::Text bestLine = MakeText(m_font, stats.str(), 15,
		sf::Color(100, 90, 84), m_finaleBounds.left + m_finaleBounds.width - 150.0f, y + 50.0f);

	l_window.Draw(titleLine);
	l_window.Draw(subtitleLine);
	l_window.Draw(gateLine);
	DrawStars(target, m_finaleBounds.left + m_finaleBounds.width - 150.0f, y + 24.0f,
		progress.bestStars, sf::Color(78, 68, 60, 150), 3001U);
	l_window.Draw(bestLine);
}

void StageSelectState::DrawSelectionDetails(Window& l_window)
{
	sf::RenderTarget& target = l_window.GetRenderWindow();
	const int levelId = GetSelectedLevelId();
	const LevelConfig& config = GetAllLevels()[static_cast<std::size_t>(levelId - 1)];
	const bool isFinale = (levelId == 10);
	const bool healed = isFinale ? m_stateManager.HasCompletedLevel(10) : m_stateManager.IsPageHealed(levelId);
	const bool available = m_stateManager.CanAccessCampaignLevel(levelId);
	const sf::Color outline = GetTileOutlineColor(false, available, healed);

	InkRenderer::DrawWobblyRect(target, m_detailBounds.left, m_detailBounds.top,
		m_detailBounds.width, m_detailBounds.height,
		sf::Color(242, 236, 228, 220), outline, 1.0f, 0.05f, 4011U);

	const float x = m_detailBounds.left + 18.0f;
	const float y = m_detailBounds.top + 12.0f;

	sf::Text title = MakeText(m_font,
		std::to_string(config.id) + ". " + config.name, 22, sf::Color(55, 45, 40), x, y);
	l_window.Draw(title);

	InkRenderer::DrawWobblyLine(target, x, y + 26.0f,
		m_detailBounds.left + m_detailBounds.width - 18.0f, y + 26.0f,
		sf::Color(140, 126, 116, 80), 1.0f, 0.08f, 4012U);

	if (isFinale)
	{
		const bool unlocked = m_stateManager.IsL10Unlocked();
		const bool completed = m_stateManager.HasCompletedLevel(10);
		const std::string gateState = completed ? "Open" : (unlocked ? "Unsealed" : "Sealed");
		const std::string lineOne = "Threat: " + config.corruptionLabel + "   Gate: " + gateState;
		const std::string lineTwo = unlocked
			? "All healed pages converge here."
			: "Heal all 8 notebook pages to break the seal. Current: " +
				std::to_string(m_stateManager.GetHealedPageCount()) + "/8";

		sf::Text info = MakeText(m_font, lineOne, 15, sf::Color(105, 88, 82), x, y + 34.0f);
		sf::Text hint = MakeText(m_font, lineTwo, 15, sf::Color(90, 82, 78), x, y + 54.0f);
		l_window.Draw(info);
		l_window.Draw(hint);
		return;
	}

	std::string rewardLine = "Reward: ???";
	if (config.abilityReward != AbilityId::None && healed)
		rewardLine = "Reward unlocked: " + std::string(GetAbilityDefinition(config.abilityReward).name);
	else if (config.abilityReward != AbilityId::None)
		rewardLine = "Reward: ???";

	const std::string lineOne = "Corruption: " + config.corruptionLabel +
		"   Hint: " + config.difficultyHint;
	const std::string lineTwo = healed
		? rewardLine + "   This page is healed."
		: (config.bossConfig.enabled && m_stateManager.GetLevelProgress(levelId).stageCleared
			? rewardLine + "   Stage cleared. Defeat the boss to heal it."
			: (config.bossConfig.enabled
				? rewardLine + "   Clear this page to confront its boss."
				: rewardLine + "   Clear this level to finish the page."));

	sf::Text info = MakeText(m_font, lineOne, 15, sf::Color(105, 88, 82), x, y + 34.0f);
	sf::Text hint = MakeText(m_font, lineTwo, 15, sf::Color(90, 82, 78), x, y + 54.0f);
	l_window.Draw(info);
	l_window.Draw(hint);
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
	status << m_stateManager.GetHealedPageCount() << "/8 pages healed"
		   << "   " << unlockedAbilityCount << " abilities unlocked"
		   << "   Equipped: " << equipped.name;
	m_statusStrip.setString(status.str());
	CenterText(m_statusStrip, m_stateManager.GetWindow().GetWindowSize().x / 2.0f, 116.0f);
}

void StageSelectState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	window.Draw(m_title);
	window.Draw(m_subtitle);
	window.Draw(m_statusStrip);

	sf::FloatRect titleBounds = m_title.getGlobalBounds();
	InkRenderer::DrawWobblyLine(target,
		titleBounds.left, titleBounds.top + titleBounds.height + 6.0f,
		titleBounds.left + titleBounds.width, titleBounds.top + titleBounds.height + 6.0f,
		sf::Color(180, 50, 40, 120), 1.5f, 0.10f, 2701U);

	for (std::size_t i = 0; i < m_tiles.size(); ++i)
		DrawStageTile(window, m_tiles[i], static_cast<int>(i) == m_selectedIndex);

	DrawFinaleGate(window, m_selectedIndex == 8);
	DrawSelectionDetails(window);
	window.Draw(m_footer);
}
