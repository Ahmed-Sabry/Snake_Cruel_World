#pragma once

#include "CutsceneAction.h"
#include "CutsceneActions.h"
#include "Easing.h"
#include <string>
#include <vector>

struct Keyframe
{
	float time;
	float value;
	EasingFunc easing = Easing::EaseOutQuad;
};

struct Keyframe2D
{
	float time;
	sf::Vector2f value;
	EasingFunc easing = Easing::EaseOutQuad;
};

class KeyframeAction : public CutsceneAction
{
public:
	KeyframeAction(const std::string& l_entityName, AnimProperty l_prop,
				   std::vector<Keyframe> l_keyframes);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;

private:
	void ApplyValue(float l_value);

	std::string m_entityName;
	AnimProperty m_property;
	std::vector<Keyframe> m_keyframes;
	float m_totalDuration = 0.f;
	float m_elapsed = 0.f;
};

class DeferredKeyframeAction : public CutsceneAction
{
public:
	using ReadFunc = std::function<float(const CutsceneEntity&)>;

	DeferredKeyframeAction(const std::string& l_entityName, AnimProperty l_prop,
						   std::vector<Keyframe> l_keyframes,
						   float l_defaultFrom, ReadFunc l_readFrom);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;

private:
	std::string m_entityName;
	AnimProperty m_prop;
	std::vector<Keyframe> m_keyframes;
	float m_defaultFrom;
	ReadFunc m_readFrom;
	std::unique_ptr<KeyframeAction> m_inner;
};

namespace KeyframeTrack
{
	CutsceneActionPtr Create(const std::string& l_name, AnimProperty l_prop,
							 std::vector<Keyframe> l_keyframes);

	CutsceneActionPtr Create2D(const std::string& l_name,
							   AnimProperty l_propX, AnimProperty l_propY,
							   std::vector<Keyframe2D> l_keyframes);
}
