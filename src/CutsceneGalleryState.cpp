#include "CutsceneGalleryState.h"
#include "CutsceneDefs.h"
#include "AudioManager.h"
#include "LevelConfig.h"
#include "InkRenderer.h"
#include <iostream>
#include <cmath>

CutsceneGalleryState::CutsceneGalleryState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_selectedItem(0),
	  m_keyReleased(true),
	  m_animTimer(0.0f)
{
}

void CutsceneGalleryState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "CutsceneGalleryState: Failed to load font" << std::endl;

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
	m_title.setString("~ Cutscene Gallery ~");
	m_title.setCharacterSize(44);
	m_title.setFillColor(sf::Color(180, 50, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 120.f);

	// Build gallery items from registry
	m_items.clear();
	auto entries = CutsceneDefs::GetAllEntries();
	float startY = 300.f;
	float spacing = 55.f;

	for (size_t i = 0; i < entries.size(); ++i)
	{
		GalleryItem item;
		item.id = entries[i].id;
		item.unlocked = entries[i].isUnlocked(m_stateManager);
		item.label = item.unlocked ? entries[i].displayName : "???";
		item.text.setFont(m_font);
		item.text.setString(item.label);
		item.text.setCharacterSize(30);
		sf::FloatRect bounds = item.text.getLocalBounds();
		float y = startY + (float)i * spacing;
		item.text.setPosition((winSize.x - bounds.width) / 2.0f, y);
		m_items.push_back(std::move(item));
	}

	// Back hint
	m_backHint.setFont(m_font);
	m_backHint.setString("[ESC] Back");
	m_backHint.setCharacterSize(18);
	m_backHint.setFillColor(sf::Color(140, 120, 110));
	sf::FloatRect hintBounds = m_backHint.getLocalBounds();
	m_backHint.setPosition((winSize.x - hintBounds.width) / 2.0f,
						   (float)winSize.y - 60.f);

	m_selectedItem = 0;
	m_keyReleased = false;
	m_animTimer = 0.0f;

	m_stateManager.GetAudio().PlayMusic("content/audio/music/menu_theme.ogg");
}

void CutsceneGalleryState::OnExit()
{
	m_stateManager.GetAudio().StopMusic();
}

void CutsceneGalleryState::HandleInput()
{
	Window& window = m_stateManager.GetWindow();
	bool upPressed = window.IsKeyPressed(sf::Keyboard::Up) ||
					 window.IsKeyPressed(sf::Keyboard::W);
	bool downPressed = window.IsKeyPressed(sf::Keyboard::Down) ||
					   window.IsKeyPressed(sf::Keyboard::S);
	bool enterPressed = window.IsKeyPressed(sf::Keyboard::Return) ||
						window.IsKeyPressed(sf::Keyboard::Space);
	bool escPressed = window.IsKeyPressed(sf::Keyboard::Escape);

	if (!upPressed && !downPressed && !enterPressed && !escPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;
	int itemCount = (int)m_items.size();

	if (escPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::MainMenu);
	}
	else if (upPressed && itemCount > 0)
	{
		m_selectedItem--;
		if (m_selectedItem < 0)
			m_selectedItem = itemCount - 1;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (downPressed && itemCount > 0)
	{
		m_selectedItem++;
		if (m_selectedItem >= itemCount)
			m_selectedItem = 0;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (enterPressed && m_selectedItem >= 0 && m_selectedItem < itemCount)
	{
		if (m_items[m_selectedItem].unlocked)
		{
			m_stateManager.GetAudio().PlaySound("menu_select");
			m_stateManager.cutsceneId = m_items[m_selectedItem].id;
			m_stateManager.cutsceneReturnState = StateType::CutsceneGallery;
			m_stateManager.cutsceneOnSkip = nullptr;
			m_stateManager.SwitchTo(StateType::Cutscene);
		}
		else
		{
			m_stateManager.GetAudio().PlaySound("menu_navigate");
		}
	}
}

void CutsceneGalleryState::Update(float l_dt)
{
	m_animTimer += l_dt;
	if (m_animTimer > 10000.0f) m_animTimer -= 10000.0f;
	int itemCount = (int)m_items.size();

	for (int i = 0; i < itemCount; i++)
	{
		auto& item = m_items[i];

		if (i == m_selectedItem)
		{
			if (item.unlocked)
			{
				float pulse = (std::sin(m_animTimer * 4.0f) + 1.0f) / 2.0f;
				int r = 160 + (int)(40 * pulse);
				int g = 40 + (int)(20 * pulse);
				int b = 30;
				item.text.setFillColor(sf::Color(r, g, b));
			}
			else
			{
				item.text.setFillColor(sf::Color(140, 120, 110));
			}
			item.text.setString("> " + item.label);
		}
		else
		{
			if (item.unlocked)
				item.text.setFillColor(sf::Color(100, 90, 85));
			else
				item.text.setFillColor(sf::Color(140, 130, 125));

			item.text.setString("  " + item.label);
		}

		// Re-center after string change
		sf::FloatRect bounds = item.text.getLocalBounds();
		float winWidth = (float)m_stateManager.GetWindow().GetWindowSize().x;
		item.text.setPosition((winWidth - bounds.width) / 2.0f, item.text.getPosition().y);
	}
}

void CutsceneGalleryState::Render()
{
	Window& window = m_stateManager.GetWindow();
	sf::RenderTarget& target = window.GetRenderWindow();

	// Paper background
	if (m_paperBg.IsGenerated())
		m_paperBg.Render(target);

	// Wobbly underline below title
	sf::FloatRect titleBounds = m_title.getGlobalBounds();
	InkRenderer::DrawWobblyLine(target,
								titleBounds.left, titleBounds.top + titleBounds.height + 8,
								titleBounds.left + titleBounds.width, titleBounds.top + titleBounds.height + 8,
								sf::Color(180, 50, 40, 150), 2.0f, 0.15f,
								(unsigned int)(m_animTimer * 0.5f));

	window.Draw(m_title);

	int itemCount = (int)m_items.size();
	for (int i = 0; i < itemCount; i++)
	{
		window.Draw(m_items[i].text);

		// Draw selection circle around selected item
		if (i == m_selectedItem)
		{
			sf::FloatRect bounds = m_items[i].text.getGlobalBounds();
			float cx = bounds.left + bounds.width * 0.5f;
			float cy = bounds.top + bounds.height * 0.5f;
			float rx = bounds.width * 0.6f + 10.0f;
			float ry = bounds.height * 0.5f + 8.0f;

			InkRenderer::DrawWobblyCircle(target, cx, cy,
										  std::max(rx, ry),
										  sf::Color::Transparent,
										  sf::Color(180, 50, 40, 80),
										  1.0f, 0.3f,
										  (unsigned int)(m_animTimer * 10.0f), 16);
		}
	}

	// Back hint
	window.Draw(m_backHint);
}
