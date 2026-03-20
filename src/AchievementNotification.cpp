#include "AchievementNotification.h"
#include "InkRenderer.h"
#include <cmath>

AchievementNotification::AchievementNotification()
	: m_phase(Phase::Idle),
	  m_timer(0.0f),
	  m_slideInDuration(0.3f),
	  m_holdDuration(2.5f),
	  m_slideOutDuration(0.3f),
	  m_fontLoaded(false)
{
}

void AchievementNotification::Init(const sf::Font& l_font)
{
	m_nameText.setFont(l_font);
	m_nameText.setCharacterSize(18);
	m_descText.setFont(l_font);
	m_descText.setCharacterSize(14);
	m_fontLoaded = true;
}

void AchievementNotification::Push(const AchievementDef& l_def)
{
	Pending p;
	p.name = l_def.name;
	p.desc = l_def.description;
	m_queue.push(std::move(p));

	if (m_phase == Phase::Idle)
		StartNext();
}

void AchievementNotification::StartNext()
{
	if (m_queue.empty())
	{
		m_phase = Phase::Idle;
		return;
	}

	auto p = m_queue.front();
	m_queue.pop();
	m_currentName = p.name;
	m_currentDesc = p.desc;
	m_phase = Phase::SlideIn;
	m_timer = 0.0f;
}

void AchievementNotification::Update(float l_dt)
{
	if (m_phase == Phase::Idle) return;

	m_timer += l_dt;

	switch (m_phase)
	{
		case Phase::SlideIn:
			if (m_timer >= m_slideInDuration)
			{
				m_phase = Phase::Hold;
				m_timer = 0.0f;
			}
			break;
		case Phase::Hold:
			if (m_timer >= m_holdDuration)
			{
				m_phase = Phase::SlideOut;
				m_timer = 0.0f;
			}
			break;
		case Phase::SlideOut:
			if (m_timer >= m_slideOutDuration)
			{
				m_timer = 0.0f;
				StartNext(); // next in queue or Idle
			}
			break;
		default:
			break;
	}
}

bool AchievementNotification::IsShowing() const
{
	return m_phase != Phase::Idle;
}

void AchievementNotification::Render(sf::RenderTarget& l_target, float l_windowWidth)
{
	if (m_phase == Phase::Idle || !m_fontLoaded) return;

	// Calculate slide offset (slides in from right)
	float boxWidth = 320.f;
	float boxHeight = 58.f;
	float boxY = 50.f;
	float slideOffset = 0.0f;

	switch (m_phase)
	{
		case Phase::SlideIn:
		{
			float t = m_timer / m_slideInDuration;
			float easeOut = 1.0f - (1.0f - t) * (1.0f - t); // quadratic ease-out
			slideOffset = (1.0f - easeOut) * (boxWidth + 20.f);
			break;
		}
		case Phase::Hold:
			slideOffset = 0.0f;
			break;
		case Phase::SlideOut:
		{
			float t = m_timer / m_slideOutDuration;
			float easeIn = t * t; // quadratic ease-in
			slideOffset = easeIn * (boxWidth + 20.f);
			break;
		}
		default:
			break;
	}

	float boxX = l_windowWidth - boxWidth - 10.f + slideOffset;

	// Background (slightly off-white paper scrap)
	sf::Color bgColor(250, 242, 228, 230);
	sf::Color borderColor(60, 50, 45, 200);
	InkRenderer::DrawWobblyRect(l_target, boxX, boxY, boxWidth, boxHeight,
								bgColor, borderColor, 1.5f, 0.08f, 999);

	// Torn bottom edge
	InkRenderer::DrawTornEdge(l_target, boxX, boxY + boxHeight, boxWidth,
							  true, false, borderColor, 0.08f, 998);

	// Star icon
	InkRenderer::DrawStar(l_target, boxX + 20.f, boxY + boxHeight * 0.5f,
						  8.0f, sf::Color(180, 140, 40, 220), borderColor,
						  0.1f, true, 997);

	// Achievement name
	m_nameText.setString(m_currentName);
	m_nameText.setFillColor(sf::Color(60, 50, 45));
	m_nameText.setPosition(boxX + 40.f, boxY + 6.f);
	l_target.draw(m_nameText);

	// Description
	m_descText.setString(m_currentDesc);
	m_descText.setFillColor(sf::Color(100, 90, 85));
	m_descText.setPosition(boxX + 40.f, boxY + 30.f);
	l_target.draw(m_descText);
}
