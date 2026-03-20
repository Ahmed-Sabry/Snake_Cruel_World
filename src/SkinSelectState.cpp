#include "SkinSelectState.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "StatsManager.h"
#include "AchievementManager.h"
#include "InkRenderer.h"
#include <iostream>
#include <cmath>

SkinSelectState::SkinSelectState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedItem(0),
	  m_keyReleased(true),
	  m_animTimer(0.0f)
{
}

bool SkinSelectState::IsSkinUnlocked(int l_skinIndex) const
{
	if (l_skinIndex <= 0) return true; // Classic (0) always unlocked
	if (l_skinIndex >= (int)m_skins.size()) return false;
	// Skin i unlocked by 3-starring level i
	int levelIdx = m_skins[l_skinIndex].unlockLevel - 1;
	if (levelIdx < 0 || levelIdx >= NUM_LEVELS) return false;
	return m_stateManager.starRatings[levelIdx] >= 3;
}

void SkinSelectState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "SkinSelectState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();

	LevelConfig menuConfig{};
	menuConfig.id = 0;
	menuConfig.paperTone = sf::Color(245, 235, 220);
	menuConfig.inkTint = sf::Color(60, 50, 45);
	menuConfig.corruption = 0.02f;
	m_paperBg.Generate(menuConfig, winSize.x, winSize.y);
	window.SetBackground(menuConfig.paperTone);

	m_title.setFont(m_font);
	m_title.setString("~ Snake Skins ~");
	m_title.setCharacterSize(40);
	m_title.setFillColor(sf::Color(180, 50, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 35.f);

	m_skins = GetAllSkins();
	m_skinLabels.clear();
	float startY = 110.f;
	float spacing = 42.f;

	for (size_t i = 0; i < m_skins.size(); i++)
	{
		sf::Text t;
		t.setFont(m_font);
		t.setCharacterSize(22);
		t.setPosition(200.f, startY + i * spacing);
		m_skinLabels.push_back(std::move(t));
	}

	m_selectedItem = m_stateManager.activeSkinIndex;
	if (m_selectedItem < 0 || m_selectedItem >= (int)m_skins.size())
		m_selectedItem = 0;

	m_keyReleased = false;
	m_animTimer = 0.0f;
}

void SkinSelectState::OnExit()
{
}

void SkinSelectState::HandleInput()
{
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	bool escPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);

	if (!upPressed && !downPressed && !enterPressed && !escPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased) return;
	m_keyReleased = false;

	int itemCount = (int)m_skins.size();

	if (escPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::MainMenu);
	}
	else if (upPressed)
	{
		m_selectedItem--;
		if (m_selectedItem < 0) m_selectedItem = itemCount - 1;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (downPressed)
	{
		m_selectedItem++;
		if (m_selectedItem >= itemCount) m_selectedItem = 0;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (enterPressed)
	{
		if (IsSkinUnlocked(m_selectedItem))
		{
			m_stateManager.activeSkinIndex = m_selectedItem;
			m_stateManager.GetAudio().PlaySound("skin_select");
			// Save immediately
			SaveManager::Save(m_stateManager, m_stateManager.GetStats(),
							  m_stateManager.GetAchievements());
		}
	}
}

void SkinSelectState::Update(float l_dt)
{
	m_animTimer += l_dt;
	if (m_animTimer > 10000.0f) m_animTimer -= 10000.0f;
	int itemCount = (int)m_skins.size();

	for (int i = 0; i < itemCount; i++)
	{
		bool unlocked = IsSkinUnlocked(i);
		bool active = (i == m_stateManager.activeSkinIndex);
		bool selected = (i == m_selectedItem);

		std::string prefix;
		if (active) prefix = ">> ";
		else if (selected) prefix = "> ";
		else prefix = "   ";

		std::string label = prefix + m_skins[i].name;
		if (!unlocked)
			label += "  (LOCKED - " + std::string(m_skins[i].description) + ")";
		else if (active)
			label += "  (ACTIVE)";

		m_skinLabels[i].setString(label);

		if (selected && unlocked)
		{
			float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
			int r = 160 + (int)(40 * pulse);
			m_skinLabels[i].setFillColor(sf::Color(r, 40, 30));
		}
		else if (unlocked)
			m_skinLabels[i].setFillColor(sf::Color(80, 70, 65));
		else
			m_skinLabels[i].setFillColor(sf::Color(150, 140, 135));

		// Center
		sf::FloatRect bounds = m_skinLabels[i].getLocalBounds();
		float winWidth = (float)m_stateManager.GetWindow().GetWindowSize().x;
		m_skinLabels[i].setPosition((winWidth - bounds.width) / 2.0f,
									m_skinLabels[i].getPosition().y);
	}
}

void SkinSelectState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	window.Draw(m_title);

	for (size_t i = 0; i < m_skinLabels.size(); i++)
	{
		window.Draw(m_skinLabels[i]);

		// Draw selection circle
		if ((int)i == m_selectedItem)
		{
			sf::FloatRect bounds = m_skinLabels[i].getGlobalBounds();
			float cx = bounds.left + bounds.width * 0.5f;
			float cy = bounds.top + bounds.height * 0.5f;
			float r = bounds.width * 0.55f + 10.0f;
			InkRenderer::DrawWobblyCircle(target, cx, cy, r,
										  sf::Color::Transparent,
										  sf::Color(180, 50, 40, 60),
										  1.0f, 0.2f,
										  (unsigned int)(m_animTimer * 8.0f), 16);
		}
	}

	// Preview: draw a small snake strip with the selected skin's colors
	float previewY = (float)window.GetWindowSize().y - 80.f;
	float previewX = (float)window.GetWindowSize().x / 2.0f - 60.f;
	float segSize = 16.f;

	if (m_selectedItem >= 0 && m_selectedItem < (int)m_skins.size() &&
		IsSkinUnlocked(m_selectedItem))
	{
		const SnakeSkin& skin = m_skins[m_selectedItem];
		sf::Color headCol = (skin.id == 0) ? sf::Color(180, 50, 40) : skin.headColor;
		sf::Color bodyCol = (skin.id == 0) ? sf::Color(140, 40, 30) : skin.bodyColor;

		for (int s = 0; s < 6; s++)
		{
			sf::Color col = (s == 0) ? headCol : bodyCol;

			// Apply skin render flags for preview
			if (skin.renderFlags & static_cast<int>(SkinRenderFlag::Rainbow))
				col = InkRenderer::HsvToRgb(std::fmod(s * 50.0f + m_animTimer * 80.0f, 360.0f), 0.8f, 0.9f);
			if (skin.renderFlags & static_cast<int>(SkinRenderFlag::Gradient))
			{
				float t = s / 5.0f;
				col.r = (sf::Uint8)(bodyCol.r + t * ((int)skin.gradientEnd.r - bodyCol.r));
				col.g = (sf::Uint8)(bodyCol.g + t * ((int)skin.gradientEnd.g - bodyCol.g));
				col.b = (sf::Uint8)(bodyCol.b + t * ((int)skin.gradientEnd.b - bodyCol.b));
			}
			if (skin.renderFlags & static_cast<int>(SkinRenderFlag::Translucent))
				col.a = 128;

			float thick = (skin.renderFlags & static_cast<int>(SkinRenderFlag::ThickOutline)) ? 2.0f : 1.0f;

			InkRenderer::DrawWobblyRect(target,
										previewX + s * (segSize + 2), previewY,
										segSize, segSize,
										col, sf::Color(60, 50, 45), thick,
										0.1f, 700 + s);
		}
	}

	// Footer
	sf::Text footer;
	footer.setFont(m_font);
	footer.setString("[Enter] Select    [ESC] Back");
	footer.setCharacterSize(16);
	footer.setFillColor(sf::Color(100, 90, 85));
	sf::FloatRect fb = footer.getLocalBounds();
	footer.setPosition((window.GetWindowSize().x - fb.width) / 2.0f,
					   (float)window.GetWindowSize().y - 30.f);
	window.Draw(footer);
}
