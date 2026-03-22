#include "KeyframeAction.h"
#include "CutsceneState.h"
#include <algorithm>
#include <cmath>

// ── KeyframeAction ───────────────────────────────────────────────

KeyframeAction::KeyframeAction(const std::string& l_entityName, AnimProperty l_prop,
							   std::vector<Keyframe> l_keyframes)
	: m_entityName(l_entityName), m_property(l_prop), m_keyframes(std::move(l_keyframes))
{
	if (!m_keyframes.empty())
		m_totalDuration = m_keyframes.back().time;
}

void KeyframeAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed = 0.f;
	if (!m_keyframes.empty())
		ApplyValue(m_keyframes.front().value);
}

bool KeyframeAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	if (m_keyframes.size() < 2 || m_totalDuration <= 0.f)
	{
		if (!m_keyframes.empty())
			ApplyValue(m_keyframes.back().value);
		return true;
	}

	m_elapsed += l_dt;
	float t = std::min(m_elapsed, m_totalDuration);

	// Find which segment we're in
	size_t seg = 0;
	for (size_t i = 1; i < m_keyframes.size(); ++i)
	{
		if (t <= m_keyframes[i].time)
		{
			seg = i - 1;
			break;
		}
		if (i == m_keyframes.size() - 1)
			seg = i - 1;
	}

	const Keyframe& kfA = m_keyframes[seg];
	const Keyframe& kfB = m_keyframes[seg + 1];

	float segDuration = kfB.time - kfA.time;
	float localT = (segDuration > 0.f) ? std::min(1.f, (t - kfA.time) / segDuration) : 1.f;
	float eased = kfB.easing(localT);
	float value = kfA.value + (kfB.value - kfA.value) * eased;

	ApplyValue(value);
	return m_elapsed >= m_totalDuration;
}

void KeyframeAction::Skip()
{
	m_elapsed = m_totalDuration;
	if (!m_keyframes.empty())
		ApplyValue(m_keyframes.back().value);
}

void KeyframeAction::ApplyValue(float l_value)
{
	AnimPropertyUtil::Apply(m_entityName, m_property, l_value);
}

// ── DeferredKeyframeAction ───────────────────────────────────────

DeferredKeyframeAction::DeferredKeyframeAction(
	const std::string& l_entityName, AnimProperty l_prop,
	std::vector<Keyframe> l_keyframes,
	float l_defaultFrom, ReadFunc l_readFrom)
	: m_entityName(l_entityName), m_prop(l_prop),
	  m_keyframes(std::move(l_keyframes)),
	  m_defaultFrom(l_defaultFrom), m_readFrom(std::move(l_readFrom))
{
}

void DeferredKeyframeAction::Start(StateManager& l_sm)
{
	float fromValue = m_defaultFrom;
	if (CutsceneState::s_active)
	{
		auto* entity = CutsceneState::s_active->GetScene().Get(m_entityName);
		if (entity)
			fromValue = m_readFrom(*entity);
	}

	// Replace first keyframe's value with the deferred one
	auto keyframes = m_keyframes;
	if (!keyframes.empty())
		keyframes.front().value = fromValue;

	m_inner = std::make_unique<KeyframeAction>(m_entityName, m_prop, std::move(keyframes));
	m_inner->Start(l_sm);
}

bool DeferredKeyframeAction::Update(float l_dt, StateManager& l_sm)
{
	if (!m_inner) return true;
	return m_inner->Update(l_dt, l_sm);
}

void DeferredKeyframeAction::Skip()
{
	if (m_inner) m_inner->Skip();
}

// ── Convenience Factories ────────────────────────────────────────

CutsceneActionPtr KeyframeTrack::Create(const std::string& l_name, AnimProperty l_prop,
										std::vector<Keyframe> l_keyframes)
{
	return std::make_unique<KeyframeAction>(l_name, l_prop, std::move(l_keyframes));
}

CutsceneActionPtr KeyframeTrack::Create2D(const std::string& l_name,
										  AnimProperty l_propX, AnimProperty l_propY,
										  std::vector<Keyframe2D> l_keyframes)
{
	// Split 2D keyframes into separate X and Y keyframe tracks
	std::vector<Keyframe> kfX, kfY;
	kfX.reserve(l_keyframes.size());
	kfY.reserve(l_keyframes.size());

	for (const auto& kf : l_keyframes)
	{
		kfX.push_back({kf.time, kf.value.x, kf.easing});
		kfY.push_back({kf.time, kf.value.y, kf.easing});
	}

	std::vector<CutsceneActionPtr> actions;
	actions.push_back(std::make_unique<KeyframeAction>(l_name, l_propX, std::move(kfX)));
	actions.push_back(std::make_unique<KeyframeAction>(l_name, l_propY, std::move(kfY)));
	return std::make_unique<ParallelAction>(std::move(actions));
}
