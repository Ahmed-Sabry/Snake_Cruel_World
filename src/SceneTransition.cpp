#include "SceneTransition.h"
#include "CutsceneState.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// ── TransitionAction ─────────────────────────────────────────────

TransitionAction::TransitionAction(TransitionType l_type, float l_duration,
								   sf::Color l_color, EasingFunc l_easing)
	: m_type(l_type), m_duration(l_duration), m_color(l_color), m_easing(l_easing)
{
}

void TransitionAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed = 0.f;
	m_progress = 0.f;
	m_snapshotValid = false;

	// Capture current frame for snapshot-based transitions
	if (CutsceneState::s_active && m_type != TransitionType::Fade)
	{
		sf::Vector2u winSize = l_sm.GetWindow().GetWindowSize();
		if (m_snapshot.create(winSize.x, winSize.y))
		{
			m_snapshot.clear(sf::Color::Transparent);
			CutsceneState::s_active->CaptureFrame(m_snapshot);
			m_snapshotValid = true;
		}
	}

	// Load shaders based on transition type
	switch (m_type)
	{
	case TransitionType::WipeLeft:
	case TransitionType::WipeRight:
	case TransitionType::WipeUp:
	case TransitionType::WipeDown:
		m_wipeLoaded = m_wipeShader.loadFromFile("content/shaders/wipe.frag", sf::Shader::Fragment);
		if (!m_wipeLoaded)
			std::cerr << "SceneTransition: failed to load wipe.frag\n";
		break;
	case TransitionType::IrisOpen:
	case TransitionType::IrisClose:
		m_irisLoaded = m_irisShader.loadFromFile("content/shaders/iris.frag", sf::Shader::Fragment);
		if (!m_irisLoaded)
			std::cerr << "SceneTransition: failed to load iris.frag\n";
		break;
	case TransitionType::Dissolve:
		m_dissolveLoaded = m_dissolveShader.loadFromFile("content/shaders/dissolve.frag", sf::Shader::Fragment);
		if (!m_dissolveLoaded)
			std::cerr << "SceneTransition: failed to load dissolve.frag\n";
		break;
	default:
		break;
	}
}

bool TransitionAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	if (m_duration <= 0.f)
	{
		m_progress = 1.f;
		return true;
	}
	m_elapsed += l_dt;
	float t = std::min(1.f, m_elapsed / m_duration);
	m_progress = m_easing(t);
	return m_elapsed >= m_duration;
}

void TransitionAction::Render(sf::RenderTarget& l_target)
{
	if (m_type == TransitionType::Fade)
	{
		RenderFade(l_target);
		return;
	}

	// For shader-based transitions, fall back to fade if shader/snapshot unavailable
	if (!m_snapshotValid)
	{
		RenderFade(l_target);
		return;
	}

	RenderShader(l_target);
}

void TransitionAction::RenderFade(sf::RenderTarget& l_target)
{
	sf::Vector2u size = l_target.getSize();
	sf::RectangleShape overlay(sf::Vector2f((float)size.x, (float)size.y));
	sf::Color c = m_color;
	c.a = (sf::Uint8)std::max(0.f, std::min(255.f, m_progress * 255.f));
	overlay.setFillColor(c);
	l_target.draw(overlay);
}

void TransitionAction::RenderShader(sf::RenderTarget& l_target)
{
	sf::Vector2u size = l_target.getSize();
	sf::Sprite snapshot(m_snapshot.getTexture());

	switch (m_type)
	{
	case TransitionType::WipeLeft:
	case TransitionType::WipeRight:
	case TransitionType::WipeUp:
	case TransitionType::WipeDown:
	{
		if (!m_wipeLoaded)
		{
			RenderFade(l_target);
			return;
		}
		sf::Vector2f direction;
		switch (m_type)
		{
		case TransitionType::WipeLeft:  direction = {-1.f, 0.f}; break;
		case TransitionType::WipeRight: direction = {1.f, 0.f}; break;
		case TransitionType::WipeUp:    direction = {0.f, -1.f}; break;
		case TransitionType::WipeDown:  direction = {0.f, 1.f}; break;
		default: direction = {1.f, 0.f}; break;
		}
		m_wipeShader.setUniform("texture", sf::Shader::CurrentTexture);
		m_wipeShader.setUniform("progress", m_progress);
		m_wipeShader.setUniform("direction", direction);
		m_wipeShader.setUniform("color", sf::Glsl::Vec4(m_color));
		l_target.draw(snapshot, &m_wipeShader);
		break;
	}
	case TransitionType::IrisOpen:
	case TransitionType::IrisClose:
	{
		if (!m_irisLoaded)
		{
			RenderFade(l_target);
			return;
		}
		float irisProgress = (m_type == TransitionType::IrisClose) ? m_progress : (1.f - m_progress);
		m_irisShader.setUniform("texture", sf::Shader::CurrentTexture);
		m_irisShader.setUniform("progress", irisProgress);
		m_irisShader.setUniform("resolution", sf::Glsl::Vec2((float)size.x, (float)size.y));
		m_irisShader.setUniform("color", sf::Glsl::Vec4(m_color));
		l_target.draw(snapshot, &m_irisShader);
		break;
	}
	case TransitionType::Dissolve:
	{
		if (!m_dissolveLoaded)
		{
			RenderFade(l_target);
			return;
		}
		m_dissolveShader.setUniform("texture", sf::Shader::CurrentTexture);
		m_dissolveShader.setUniform("progress", m_progress);
		m_dissolveShader.setUniform("resolution", sf::Glsl::Vec2((float)size.x, (float)size.y));
		m_dissolveShader.setUniform("color", sf::Glsl::Vec4(m_color));
		l_target.draw(snapshot, &m_dissolveShader);
		break;
	}
	default:
		RenderFade(l_target);
		break;
	}
}

void TransitionAction::Skip()
{
	m_elapsed = m_duration;
	m_progress = 1.f;
}

// ── Convenience Factory ──────────────────────────────────────────

CutsceneActionPtr Transition::Create(TransitionType l_type, float l_duration,
									 sf::Color l_color, EasingFunc l_easing)
{
	return std::make_unique<TransitionAction>(l_type, l_duration, l_color, l_easing);
}

TransitionType Transition::FromName(const std::string& l_name)
{
	if (l_name == "fade")       return TransitionType::Fade;
	if (l_name == "wipeLeft")   return TransitionType::WipeLeft;
	if (l_name == "wipeRight")  return TransitionType::WipeRight;
	if (l_name == "wipeUp")     return TransitionType::WipeUp;
	if (l_name == "wipeDown")   return TransitionType::WipeDown;
	if (l_name == "irisOpen")   return TransitionType::IrisOpen;
	if (l_name == "irisClose")  return TransitionType::IrisClose;
	if (l_name == "dissolve")   return TransitionType::Dissolve;
	return TransitionType::Fade;
}
