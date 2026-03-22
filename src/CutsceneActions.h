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
	void Start(StateManager& l_sm) override;
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
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	bool IsPersistent() const override { return m_toBlack; }
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
	bool IsPersistent() const override { return true; }
	void Skip() override;

private:
	std::string m_fullText;
	sf::Vector2f m_position;
	unsigned int m_charSize;
	sf::Color m_color;
	float m_charsPerSec;
	bool m_waitForInputConfigured;
	bool m_waitForInput;

	sf::Font* m_fontPtr = nullptr;
	sf::Text m_text;
	float m_elapsed = 0.f;
	int m_visibleChars = 0;
	bool m_textComplete = false;
	bool m_inputReceived = false;
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
private:
	std::vector<CutsceneActionPtr> m_actions;
	std::vector<bool> m_done;
};

// ── Phase 3: Animation Actions ────────────────────────────────────

enum class AnimProperty { PositionX, PositionY, ScaleX, ScaleY, Rotation, Alpha, ColorR, ColorG, ColorB, Corruption };

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
	void Start(StateManager& l_sm) override;
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

// Deferred animation helpers — read entity's current value at Start() time

class DeferredSingleAnimAction : public CutsceneAction
{
public:
	using ReadFunc = std::function<float(const CutsceneEntity&)>;
	DeferredSingleAnimAction(const std::string& l_name, AnimProperty l_prop,
							 float l_target, float l_defaultFrom,
							 float l_duration, EasingFunc l_easing,
							 ReadFunc l_readFrom);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;
private:
	std::string m_name;
	AnimProperty m_prop;
	float m_target, m_defaultFrom, m_duration;
	EasingFunc m_easing;
	ReadFunc m_readFrom;
	std::unique_ptr<AnimateAction> m_inner;
};

class DeferredDualAnimAction : public CutsceneAction
{
public:
	using ReadFunc = std::function<sf::Vector2f(const CutsceneEntity&)>;
	DeferredDualAnimAction(const std::string& l_name,
						   AnimProperty l_propX, AnimProperty l_propY,
						   sf::Vector2f l_target, sf::Vector2f l_defaultFrom,
						   float l_duration, EasingFunc l_easing,
						   ReadFunc l_readFrom);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	void Skip() override;
private:
	std::string m_name;
	AnimProperty m_propX, m_propY;
	sf::Vector2f m_target, m_defaultFrom;
	float m_duration;
	EasingFunc m_easing;
	ReadFunc m_readFrom;
	std::unique_ptr<ParallelAction> m_inner;
};

// Convenience factories
namespace MoveAction
{
	CutsceneActionPtr Create(const std::string& l_name, sf::Vector2f l_target,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

namespace ScaleToAction
{
	CutsceneActionPtr Create(const std::string& l_name, sf::Vector2f l_target,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

namespace FadeEntityAction
{
	CutsceneActionPtr Create(const std::string& l_name, float l_targetAlpha,
							 float l_duration, EasingFunc l_easing = Easing::EaseOutQuad);
}

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

class AnimatePostProcessAction : public CutsceneAction
{
public:
	AnimatePostProcessAction(float l_fromCorruption, float l_toCorruption,
							 float l_duration, EasingFunc l_easing,
							 bool l_inkBleed = false, bool l_chromatic = false,
							 bool l_psychedelic = false);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;
private:
	LevelConfig MakeConfig(float corruption) const;
	float m_fromCorruption, m_toCorruption, m_duration;
	EasingFunc m_easing;
	bool m_inkBleed, m_chromatic, m_psychedelic;
	float m_elapsed = 0.f;
};

class ClearPersistentAction : public CutsceneAction
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
