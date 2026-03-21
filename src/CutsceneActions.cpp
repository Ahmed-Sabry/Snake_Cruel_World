#include "CutsceneActions.h"
#include "CutsceneState.h"
#include "StateManager.h"
#include "AudioManager.h"
#include "ParticleSystem.h"
#include "ScreenShake.h"
#include "PaperBackground.h"
#include "PostProcessor.h"
#include <algorithm>
#include <cmath>

// Sentinel level ID for cutscene-generated configs (avoids colliding with real levels)
static constexpr int CUTSCENE_LEVEL_ID = -1;

// ── WaitAction ────────────────────────────────────────────────────

WaitAction::WaitAction(float l_duration) : m_duration(l_duration) {}

void WaitAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed = 0.f;
}

bool WaitAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed += l_dt;
	return m_elapsed >= m_duration;
}

void WaitAction::Skip() { m_elapsed = m_duration; }

// ── FadeAction ────────────────────────────────────────────────────

FadeAction::FadeAction(bool l_toBlack, float l_duration, sf::Color l_color)
	: m_toBlack(l_toBlack), m_duration(l_duration), m_color(l_color)
{
	m_currentAlpha = m_toBlack ? 0.f : 255.f;
}

void FadeAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed = 0.f;
	m_currentAlpha = m_toBlack ? 0.f : 255.f;
}

bool FadeAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed += l_dt;
	float t = std::min(1.f, m_elapsed / m_duration);
	float eased = Easing::EaseOutQuad(t);
	m_currentAlpha = m_toBlack ? eased * 255.f : (1.f - eased) * 255.f;
	return m_elapsed >= m_duration;
}

void FadeAction::Render(sf::RenderTarget& l_target)
{
	sf::Vector2u size = l_target.getSize();
	sf::RectangleShape overlay(sf::Vector2f((float)size.x, (float)size.y));
	sf::Color c = m_color;
	c.a = (sf::Uint8)std::max(0.f, std::min(255.f, m_currentAlpha));
	overlay.setFillColor(c);
	l_target.draw(overlay);
}

void FadeAction::Skip()
{
	m_elapsed = m_duration;
	m_currentAlpha = m_toBlack ? 255.f : 0.f;
}

// ── TypewriterTextAction ──────────────────────────────────────────

TypewriterTextAction::TypewriterTextAction(const std::string& l_text,
										   sf::Vector2f l_position,
										   unsigned int l_charSize,
										   sf::Color l_color,
										   float l_charsPerSec,
										   bool l_waitForInput)
	: m_fullText(l_text), m_position(l_position), m_charSize(l_charSize),
	  m_color(l_color), m_charsPerSec(l_charsPerSec), m_waitForInput(l_waitForInput)
{
}

void TypewriterTextAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_fontPtr = CutsceneState::s_active ? &CutsceneState::s_active->GetFont() : nullptr;
	if (m_fontPtr)
	{
		m_text.setFont(*m_fontPtr);
		m_text.setCharacterSize(m_charSize);
		m_text.setFillColor(m_color);
		m_text.setPosition(m_position);
	}
	m_elapsed = 0.f;
	m_visibleChars = 0;
	m_textComplete = false;
	m_inputReceived = false;
	m_cursorTimer = 0.f;
}

bool TypewriterTextAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	m_cursorTimer += l_dt;

	if (!m_textComplete)
	{
		m_elapsed += l_dt;
		m_visibleChars = std::min((int)m_fullText.size(),
								 (int)(m_elapsed * m_charsPerSec));
		if (m_visibleChars >= (int)m_fullText.size())
			m_textComplete = true;
	}

	if (m_textComplete)
	{
		if (!m_waitForInput)
			return true;

		// Wait for Enter/Space press (require release-then-press cycle)
		bool pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) ||
					   sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
		if (!pressed)
			m_inputReceived = false; // key released — ready for next press
		else if (!m_inputReceived)
		{
			m_inputReceived = true;
			return true; // fresh press detected — advance
		}
		// else: key still held from previous press/skip — wait for release
	}

	return false;
}

void TypewriterTextAction::Render(sf::RenderTarget& l_target)
{
	if (!m_fontPtr)
		return;

	std::string display = m_fullText.substr(0, m_visibleChars);

	// Blinking cursor while typing or waiting for input
	if (!m_textComplete || (m_waitForInput && !m_inputReceived))
	{
		bool cursorOn = std::fmod(m_cursorTimer, 0.8f) < 0.4f;
		if (cursorOn)
			display += "|";
	}

	m_text.setString(display);
	l_target.draw(m_text);
}

void TypewriterTextAction::Skip()
{
	if (!m_textComplete)
	{
		// First skip: complete the text, but require a fresh key press to advance
		m_visibleChars = (int)m_fullText.size();
		m_textComplete = true;
		// Force key-release requirement so the same press doesn't also advance
		m_inputReceived = true; // treat current press as "consumed"
	}
	else if (m_waitForInput)
	{
		// Second skip: text already complete, force advance
		m_inputReceived = true;
		m_waitForInput = false; // let Update return true
	}
}

// ── WaitForInputAction ────────────────────────────────────────────

WaitForInputAction::WaitForInputAction() {}

void WaitForInputAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_fontPtr = CutsceneState::s_active ? &CutsceneState::s_active->GetFont() : nullptr;
	if (m_fontPtr)
	{
		m_prompt.setFont(*m_fontPtr);
		m_prompt.setString("Press Enter to continue...");
		m_prompt.setCharacterSize(16);
		m_prompt.setFillColor(sf::Color(120, 100, 90, 150));
	}
	m_done = false;
	m_timer = 0.f;
	m_keyWasUp = false;
}

bool WaitForInputAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;

	// Skip() was called — advance immediately
	if (m_done)
		return true;

	m_timer += l_dt;

	bool pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) ||
				   sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	if (!pressed)
		m_keyWasUp = true;
	else if (m_keyWasUp)
	{
		m_done = true;
		return true;
	}

	return false;
}

void WaitForInputAction::Render(sf::RenderTarget& l_target)
{
	if (!m_fontPtr)
		return;

	sf::Vector2u size = l_target.getSize();
	float alpha = std::fmod(m_timer, 1.2f) < 0.6f ? 150.f : 80.f;
	sf::Color c = m_prompt.getFillColor();
	c.a = (sf::Uint8)alpha;
	m_prompt.setFillColor(c);

	sf::FloatRect bounds = m_prompt.getLocalBounds();
	m_prompt.setPosition((size.x - bounds.width) / 2.f, size.y - 50.f);
	l_target.draw(m_prompt);
}

void WaitForInputAction::Skip() { m_done = true; }

// ── ParallelAction ────────────────────────────────────────────────

ParallelAction::ParallelAction(std::vector<CutsceneActionPtr> l_actions)
	: m_actions(std::move(l_actions))
{
}

void ParallelAction::Start(StateManager& l_sm)
{
	m_done.assign(m_actions.size(), false);
	for (auto& action : m_actions)
		action->Start(l_sm);
}

bool ParallelAction::Update(float l_dt, StateManager& l_sm)
{
	bool allDone = true;
	for (size_t i = 0; i < m_actions.size(); ++i)
	{
		if (!m_done[i])
		{
			m_done[i] = m_actions[i]->Update(l_dt, l_sm);
			if (!m_done[i])
				allDone = false;
		}
	}
	return allDone;
}

void ParallelAction::Render(sf::RenderTarget& l_target)
{
	for (size_t i = 0; i < m_actions.size(); ++i)
	{
		// Skip finished non-persistent children
		if (m_done[i] && !m_actions[i]->IsPersistent())
			continue;
		m_actions[i]->Render(l_target);
	}
}

bool ParallelAction::IsPersistent() const
{
	for (const auto& action : m_actions)
	{
		if (action->IsPersistent())
			return true;
	}
	return false;
}

void ParallelAction::Skip()
{
	for (auto& action : m_actions)
		action->Skip();
}

// ── SpawnEntityAction ─────────────────────────────────────────────

SpawnEntityAction::SpawnEntityAction(const std::string& l_name, EntityShape l_shape,
									 SetupFunc l_setup)
	: m_name(l_name), m_shape(l_shape), m_setup(std::move(l_setup))
{
}

void SpawnEntityAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (CutsceneState::s_active)
	{
		auto& entity = CutsceneState::s_active->GetScene().Spawn(m_name, m_shape);
		if (m_setup)
			m_setup(entity);
	}
}

bool SpawnEntityAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── DestroyEntityAction ───────────────────────────────────────────

DestroyEntityAction::DestroyEntityAction(const std::string& l_name)
	: m_name(l_name) {}

void DestroyEntityAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (CutsceneState::s_active)
		CutsceneState::s_active->GetScene().Destroy(m_name);
}

bool DestroyEntityAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── AnimateAction ─────────────────────────────────────────────────

AnimateAction::AnimateAction(const std::string& l_entityName, AnimProperty l_prop,
							 float l_from, float l_to, float l_duration,
							 EasingFunc l_easing)
	: m_entityName(l_entityName), m_property(l_prop),
	  m_from(l_from), m_to(l_to), m_duration(l_duration), m_easing(l_easing)
{
}

void AnimateAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed = 0.f;
}

bool AnimateAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	if (m_duration <= 0.f)
	{
		ApplyValue(m_to);
		return true;
	}
	m_elapsed += l_dt;
	float t = std::min(1.f, m_elapsed / m_duration);
	float eased = m_easing(t);
	float value = m_from + (m_to - m_from) * eased;
	ApplyValue(value);
	return m_elapsed >= m_duration;
}

void AnimateAction::Skip()
{
	m_elapsed = m_duration;
	ApplyValue(m_to);
}

void AnimateAction::ApplyValue(float l_value)
{
	if (!CutsceneState::s_active)
		return;
	CutsceneEntity* entity = CutsceneState::s_active->GetScene().Get(m_entityName);
	if (!entity)
		return;

	switch (m_property)
	{
	case AnimProperty::PositionX: entity->position.x = l_value; break;
	case AnimProperty::PositionY: entity->position.y = l_value; break;
	case AnimProperty::ScaleX:    entity->scale.x = l_value; break;
	case AnimProperty::ScaleY:    entity->scale.y = l_value; break;
	case AnimProperty::Rotation:  entity->rotation = l_value; break;
	case AnimProperty::Alpha:     entity->alpha = l_value; break;
	default: break;
	}
}

// ── DeferredSingleAnimAction ──────────────────────────────────────

DeferredSingleAnimAction::DeferredSingleAnimAction(
	const std::string& l_name, AnimProperty l_prop,
	float l_target, float l_defaultFrom,
	float l_duration, EasingFunc l_easing, ReadFunc l_readFrom)
	: m_name(l_name), m_prop(l_prop), m_target(l_target),
	  m_defaultFrom(l_defaultFrom), m_duration(l_duration),
	  m_easing(l_easing), m_readFrom(std::move(l_readFrom))
{
}

void DeferredSingleAnimAction::Start(StateManager& l_sm)
{
	float from = m_defaultFrom;
	if (CutsceneState::s_active)
	{
		auto* entity = CutsceneState::s_active->GetScene().Get(m_name);
		if (entity)
			from = m_readFrom(*entity);
	}
	m_inner = std::make_unique<AnimateAction>(
		m_name, m_prop, from, m_target, m_duration, m_easing);
	m_inner->Start(l_sm);
}

bool DeferredSingleAnimAction::Update(float l_dt, StateManager& l_sm)
{
	return m_inner->Update(l_dt, l_sm);
}

void DeferredSingleAnimAction::Skip()
{
	if (m_inner) m_inner->Skip();
}

// ── DeferredDualAnimAction ────────────────────────────────────────

DeferredDualAnimAction::DeferredDualAnimAction(
	const std::string& l_name,
	AnimProperty l_propX, AnimProperty l_propY,
	sf::Vector2f l_target, sf::Vector2f l_defaultFrom,
	float l_duration, EasingFunc l_easing, ReadFunc l_readFrom)
	: m_name(l_name), m_propX(l_propX), m_propY(l_propY),
	  m_target(l_target), m_defaultFrom(l_defaultFrom),
	  m_duration(l_duration), m_easing(l_easing),
	  m_readFrom(std::move(l_readFrom))
{
}

void DeferredDualAnimAction::Start(StateManager& l_sm)
{
	sf::Vector2f from = m_defaultFrom;
	if (CutsceneState::s_active)
	{
		auto* entity = CutsceneState::s_active->GetScene().Get(m_name);
		if (entity)
			from = m_readFrom(*entity);
	}
	std::vector<CutsceneActionPtr> actions;
	actions.push_back(std::make_unique<AnimateAction>(
		m_name, m_propX, from.x, m_target.x, m_duration, m_easing));
	actions.push_back(std::make_unique<AnimateAction>(
		m_name, m_propY, from.y, m_target.y, m_duration, m_easing));
	m_inner = std::make_unique<ParallelAction>(std::move(actions));
	m_inner->Start(l_sm);
}

bool DeferredDualAnimAction::Update(float l_dt, StateManager& l_sm)
{
	return m_inner->Update(l_dt, l_sm);
}

void DeferredDualAnimAction::Render(sf::RenderTarget& l_target)
{
	if (m_inner) m_inner->Render(l_target);
}

void DeferredDualAnimAction::Skip()
{
	if (m_inner) m_inner->Skip();
}

// ── Convenience Action Factories ──────────────────────────────────

CutsceneActionPtr MoveAction::Create(const std::string& l_name, sf::Vector2f l_target,
									  float l_duration, EasingFunc l_easing)
{
	return std::make_unique<DeferredDualAnimAction>(
		l_name, AnimProperty::PositionX, AnimProperty::PositionY,
		l_target, sf::Vector2f{0.f, 0.f}, l_duration, l_easing,
		[](const CutsceneEntity& e) { return e.position; });
}

CutsceneActionPtr ScaleToAction::Create(const std::string& l_name, sf::Vector2f l_target,
										 float l_duration, EasingFunc l_easing)
{
	return std::make_unique<DeferredDualAnimAction>(
		l_name, AnimProperty::ScaleX, AnimProperty::ScaleY,
		l_target, sf::Vector2f{1.f, 1.f}, l_duration, l_easing,
		[](const CutsceneEntity& e) { return e.scale; });
}

CutsceneActionPtr FadeEntityAction::Create(const std::string& l_name, float l_targetAlpha,
											float l_duration, EasingFunc l_easing)
{
	return std::make_unique<DeferredSingleAnimAction>(
		l_name, AnimProperty::Alpha, l_targetAlpha, 255.f, l_duration, l_easing,
		[](const CutsceneEntity& e) { return e.alpha; });
}

CutsceneActionPtr RotateAction::Create(const std::string& l_name, float l_targetDeg,
										float l_duration, EasingFunc l_easing)
{
	return std::make_unique<DeferredSingleAnimAction>(
		l_name, AnimProperty::Rotation, l_targetDeg, 0.f, l_duration, l_easing,
		[](const CutsceneEntity& e) { return e.rotation; });
}

// ── SoundAction ───────────────────────────────────────────────────

SoundAction::SoundAction(const std::string& l_soundName) : m_soundName(l_soundName) {}

void SoundAction::Start(StateManager& l_sm)
{
	l_sm.GetAudio().PlaySound(m_soundName);
}

bool SoundAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── ShakeAction ───────────────────────────────────────────────────

ShakeAction::ShakeAction(float l_duration, float l_intensity)
	: m_duration(l_duration), m_intensity(l_intensity) {}

void ShakeAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (CutsceneState::s_active)
		CutsceneState::s_active->GetScreenShake().Trigger(m_duration, m_intensity);
}

bool ShakeAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── ParticleAction ────────────────────────────────────────────────

ParticleAction::ParticleAction(ParticleSpawnType l_type, sf::Vector2f l_pos,
							   sf::Color l_color, int l_count)
	: m_type(l_type), m_pos(l_pos), m_color(l_color), m_count(l_count) {}

void ParticleAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (!CutsceneState::s_active)
		return;

	auto& particles = CutsceneState::s_active->GetParticles();
	switch (m_type)
	{
	case ParticleSpawnType::InkSplat:
		particles.SpawnInkSplat(m_pos, m_color, m_count);
		break;
	case ParticleSpawnType::InkDrips:
		particles.SpawnInkDrips(m_pos, m_color, m_count);
		break;
	case ParticleSpawnType::InkDust:
		particles.SpawnInkDust(m_pos, m_color, m_count);
		break;
	default: break;
	}
}

bool ParticleAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── SetBackgroundAction ───────────────────────────────────────────

SetBackgroundAction::SetBackgroundAction(sf::Color l_paperTone, sf::Color l_inkTint,
										 float l_corruption)
	: m_paperTone(l_paperTone), m_inkTint(l_inkTint), m_corruption(l_corruption) {}

void SetBackgroundAction::Start(StateManager& l_sm)
{
	if (!CutsceneState::s_active)
		return;

	LevelConfig config{};
	config.id = CUTSCENE_LEVEL_ID;
	config.paperTone = m_paperTone;
	config.inkTint = m_inkTint;
	config.corruption = m_corruption;

	sf::Vector2u winSize = l_sm.GetWindow().GetWindowSize();
	CutsceneState::s_active->GetPaperBg().Generate(config, winSize.x, winSize.y);
}

bool SetBackgroundAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── PostProcessAction ─────────────────────────────────────────────

PostProcessAction::PostProcessAction(float l_corruption, bool l_inkBleed,
									 bool l_chromatic, bool l_psychedelic)
	: m_corruption(l_corruption), m_inkBleed(l_inkBleed),
	  m_chromatic(l_chromatic), m_psychedelic(l_psychedelic) {}

void PostProcessAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (!CutsceneState::s_active)
		return;

	LevelConfig config{};
	config.corruption = m_corruption;
	config.enableInkBleed = m_inkBleed;
	config.enableChromatic = m_chromatic;
	config.enablePsychedelic = m_psychedelic;
	CutsceneState::s_active->GetPostProcessor().Configure(config);
}

bool PostProcessAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── ClearPersistentAction ─────────────────────────────────────────

void ClearPersistentAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (CutsceneState::s_active)
		CutsceneState::s_active->ClearAllPersistent();
}

bool ClearPersistentAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}

// ── LambdaAction ──────────────────────────────────────────────────

LambdaAction::LambdaAction(std::function<void(StateManager&)> l_func)
	: m_func(std::move(l_func)) {}

void LambdaAction::Start(StateManager& l_sm)
{
	if (m_func && !m_executed)
	{
		m_func(l_sm);
		m_executed = true;
	}
}

bool LambdaAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	return true;
}
