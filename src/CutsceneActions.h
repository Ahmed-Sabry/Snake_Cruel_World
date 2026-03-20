#pragma once

#include "CutsceneAction.h"
#include "Easing.h"
#include "CutsceneEntity.h"
#include "LevelConfig.h"
#include <string>
#include <vector>
#include <functional>

class StateManager;

// ── Phase 2: Core Actions ─────────────────────────────────────────

class WaitAction : public CutsceneAction
{
public:
	explicit WaitAction(float l_duration);
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;
private:
	float m_duration;
	float m_elapsed = 0.f;
};

class FadeAction : public CutsceneAction
{
public:
	FadeAction(bool l_toBlack, float l_duration, sf::Color l_color = sf::Color::Black);
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	bool IsPersistent() const override { return m_toBlack; } // only fade-to-black needs to persist
	void Skip() override;
private:
	bool m_toBlack; // true = transparent→opaque, false = opaque→transparent
	float m_duration;
	sf::Color m_color;
	float m_elapsed = 0.f;
	float m_currentAlpha = 0.f;
};

class TypewriterTextAction : public CutsceneAction
{
public:
	TypewriterTextAction(const std::string& l_text, sf::Vector2f l_position,
						 unsigned int l_charSize, sf::Color l_color,
						 float l_charsPerSec = 20.f, bool l_waitForInput = true);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	bool IsPersistent() const override { return m_persistent; }
	void Skip() override;

	void ClearPersistence() { m_persistent = false; }

private:
	std::string m_fullText;
	sf::Vector2f m_position;
	unsigned int m_charSize;
	sf::Color m_color;
	float m_charsPerSec;
	bool m_waitForInput;

	sf::Font* m_fontPtr = nullptr;
	sf::Text m_text;
	float m_elapsed = 0.f;
	int m_visibleChars = 0;
	bool m_textComplete = false;
	bool m_inputReceived = false;
	bool m_persistent = true;
	float m_cursorTimer = 0.f;
};

class WaitForInputAction : public CutsceneAction
{
public:
	WaitForInputAction();
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	void Skip() override;
private:
	sf::Font* m_fontPtr = nullptr;
	sf::Text m_prompt;
	bool m_done = false;
	float m_timer = 0.f;
	bool m_keyWasUp = false;
};

class ParallelAction : public CutsceneAction
{
public:
	explicit ParallelAction(std::vector<CutsceneActionPtr> l_actions);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	bool IsPersistent() const override;
	void Skip() override;
	void ClearChildPersistentText();
private:
	std::vector<CutsceneActionPtr> m_actions;
	std::vector<bool> m_done;
};

// ── Phase 3: Animation Actions ────────────────────────────────────

enum class AnimProperty { PositionX, PositionY, ScaleX, ScaleY, Rotation, Alpha };

class SpawnEntityAction : public CutsceneAction
{
public:
	using SetupFunc = std::function<void(CutsceneEntity&)>;
	SpawnEntityAction(const std::string& l_name, EntityShape l_shape,
					  SetupFunc l_setup = nullptr);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	std::string m_name;
	EntityShape m_shape;
	SetupFunc m_setup;
};

class DestroyEntityAction : public CutsceneAction
{
public:
	explicit DestroyEntityAction(const std::string& l_name);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	std::string m_name;
};

class AnimateAction : public CutsceneAction
{
public:
	AnimateAction(const std::string& l_entityName, AnimProperty l_prop,
				  float l_from, float l_to, float l_duration,
				  EasingFunc l_easing = Easing::EaseOutQuad);
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;
private:
	std::string m_entityName;
	AnimProperty m_property;
	float m_from, m_to, m_duration;
	float m_elapsed = 0.f;
	EasingFunc m_easing;

	void ApplyValue(float l_value);
};

// Convenience: moves entity to target position over duration
namespace MoveAction
{
	CutsceneActionPtr Create(const std::string& l_name, sf::Vector2f l_target,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

// Convenience: scales entity to target over duration
namespace ScaleToAction
{
	CutsceneActionPtr Create(const std::string& l_name, sf::Vector2f l_target,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

// Convenience: fades entity alpha to target over duration
namespace FadeEntityAction
{
	CutsceneActionPtr Create(const std::string& l_name, float l_targetAlpha,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

// Convenience: rotates entity to target degrees over duration
namespace RotateAction
{
	CutsceneActionPtr Create(const std::string& l_name, float l_targetDeg,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

// ── Phase 4: Effect Actions ───────────────────────────────────────

class SoundAction : public CutsceneAction
{
public:
	explicit SoundAction(const std::string& l_soundName);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	std::string m_soundName;
};

class ShakeAction : public CutsceneAction
{
public:
	ShakeAction(float l_duration, float l_intensity);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	float m_duration;
	float m_intensity;
};

enum class ParticleSpawnType { InkSplat, InkDrips, InkDust };

class ParticleAction : public CutsceneAction
{
public:
	ParticleAction(ParticleSpawnType l_type, sf::Vector2f l_pos,
				   sf::Color l_color, int l_count = 10);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	ParticleSpawnType m_type;
	sf::Vector2f m_pos;
	sf::Color m_color;
	int m_count;
};

class SetBackgroundAction : public CutsceneAction
{
public:
	SetBackgroundAction(sf::Color l_paperTone, sf::Color l_inkTint, float l_corruption);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	sf::Color m_paperTone;
	sf::Color m_inkTint;
	float m_corruption;
};

class PostProcessAction : public CutsceneAction
{
public:
	PostProcessAction(float l_corruption, bool l_inkBleed = false,
					  bool l_chromatic = false, bool l_psychedelic = false);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	float m_corruption;
	bool m_inkBleed;
	bool m_chromatic;
	bool m_psychedelic;
};

class ClearTextAction : public CutsceneAction
{
public:
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
};

class LambdaAction : public CutsceneAction
{
public:
	explicit LambdaAction(std::function<void(StateManager&)> l_func);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
private:
	std::function<void(StateManager&)> m_func;
	bool m_executed = false;
};
