#include "Boss.h"

#include <algorithm>

Boss::Boss(const BossConfig& l_config)
	: m_config(l_config),
	  m_state(BossLifecycleState::Dormant),
	  m_phaseIndex(0),
	  m_progressCurrent(0),
	  m_progressMax(std::max(0, l_config.progressMax)),
	  m_invulnerable(false),
	  m_stateElapsedSec(0.0f)
{
}

bool Boss::CanStartEncounter(const BossContext&) const
{
	return m_config.enabled && m_config.trigger != BossEncounterTrigger::None &&
		m_config.progressMax > 0;
}

void Boss::BeginEncounter(const BossContext&)
{
	if (m_config.progressMax <= 0)
		return;

	m_phaseIndex = 0;
	m_progressCurrent = 0;
	m_progressMax = std::max(0, m_config.progressMax);
	m_invulnerable = true;
	SetLifecycleState(BossLifecycleState::TransitionIn);
}

void Boss::Update(float l_dt, const BossContext&)
{
	m_stateElapsedSec += std::max(0.0f, l_dt);

	switch (m_state)
	{
		case BossLifecycleState::TransitionIn:
			if (m_stateElapsedSec >= m_config.transitionInDurationSec)
			{
				m_invulnerable = true;
				SetLifecycleState(BossLifecycleState::Intro);
			}
			break;
		case BossLifecycleState::Intro:
			if (m_stateElapsedSec >= m_config.introDurationSec)
			{
				m_invulnerable = false;
				SetLifecycleState(BossLifecycleState::Active);
			}
			break;
		case BossLifecycleState::Defeated:
			if (m_stateElapsedSec >= m_config.defeatDurationSec)
				SetLifecycleState(BossLifecycleState::Resolved);
			break;
		default:
			break;
	}
}

void Boss::RenderTo(sf::RenderTarget&, float, const BossContext&) const
{
}

bool Boss::CanAcceptProgressEvent(const BossProgressEvent&, const BossContext&) const
{
	return m_state == BossLifecycleState::Active && !m_invulnerable;
}

BossProgressResult Boss::ApplyProgress(const BossProgressEvent& l_event, const BossContext& l_ctx)
{
	BossProgressResult result{};
	result.progressCurrent = m_progressCurrent;
	result.phaseIndex = m_phaseIndex;

	if (!CanAcceptProgressEvent(l_event, l_ctx))
		return result;

	const int delta = std::max(0, EvaluateProgressDelta(l_event, l_ctx));
	if (delta <= 0 || m_progressMax <= 0)
		return result;

	m_progressCurrent = std::clamp(m_progressCurrent + delta, 0, m_progressMax);
	result.progressApplied = true;
	result.progressCurrent = m_progressCurrent;

	if (l_event.type == BossProgressEventType::AbilityActivated ||
		l_event.interactionType == BossInteractionType::CounterHit)
		OnAbilityInteraction(l_event.ability, l_event.interactionType, l_ctx);

	if (m_progressCurrent >= m_progressMax)
	{
		m_invulnerable = true;
		SetLifecycleState(BossLifecycleState::Defeated);
		OnDefeated(l_ctx);
		result.defeated = true;
		return result;
	}

	if (ShouldAdvancePhase(l_ctx))
	{
		AdvancePhase(l_ctx);
		result.phaseAdvanced = true;
		result.phaseIndex = m_phaseIndex;
	}

	return result;
}

bool Boss::CanBeDamagedByAbility(AbilityId l_ability, const BossContext&) const
{
	return m_state == BossLifecycleState::Active && !IsInvulnerable() &&
		l_ability != AbilityId::None &&
		l_ability == m_config.counterAbility;
}

void Boss::OnAbilityInteraction(AbilityId, BossInteractionType, const BossContext&)
{
}

void Boss::OnDefeated(const BossContext&)
{
}

BossRewardHandoff Boss::BuildRewardHandoff(const BossContext&) const
{
	BossRewardHandoff handoff{};
	handoff.healPage = m_config.healPageOnDefeat;
	handoff.cutsceneId = m_config.rewardCutsceneId;
	return handoff;
}

void Boss::SetLifecycleState(BossLifecycleState l_state)
{
	m_state = l_state;
	m_stateElapsedSec = 0.0f;
}

void Boss::SetProgressMax(int l_progressMax)
{
	m_progressMax = std::max(0, l_progressMax);
	m_progressCurrent = std::clamp(m_progressCurrent, 0, m_progressMax);
}

void Boss::SetInvulnerable(bool l_invulnerable)
{
	m_invulnerable = l_invulnerable;
}

void Boss::MarkResolved()
{
	SetLifecycleState(BossLifecycleState::Resolved);
}

int Boss::EvaluateProgressDelta(const BossProgressEvent& l_event, const BossContext& l_ctx) const
{
	if (l_event.type == BossProgressEventType::AppleCollected)
		return std::max(1, l_event.amount);

	if (l_event.type == BossProgressEventType::AbilityActivated ||
		l_event.interactionType == BossInteractionType::CounterHit)
	{
		if (!CanBeDamagedByAbility(l_event.ability, l_ctx))
			return 0;
		return std::max(1, m_config.strongCounterProgress);
	}

	return 0;
}

bool Boss::ShouldAdvancePhase(const BossContext&) const
{
	return m_state == BossLifecycleState::Active &&
		m_progressCurrent > m_phaseIndex &&
		m_progressCurrent < m_progressMax;
}

void Boss::AdvancePhase(const BossContext&)
{
	m_phaseIndex = std::min(m_progressCurrent, std::max(0, m_progressMax - 1));
}
