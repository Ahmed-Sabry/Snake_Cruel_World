#include "Ability.h"

#include <algorithm>
#include <cassert>

namespace
{
	const AbilityDefinition kNoneAbilityDefinition = {
		AbilityId::None, "No Ability", 0, 0.0f, 0.0f, 0.0f, {}
	};

	const std::array<AbilityDefinition, ABILITY_COUNT> kAbilityDefinitions = {{
		{
			AbilityId::InkFlare, "Ink Flare", 2,
			10.0f, 3.0f, 0.0f,
			{ sf::Color(255, 244, 190), sf::Color(255, 226, 120), sf::Color(255, 232, 150),
			  sf::Color::Transparent, false, false, true, false }
		},
		{
			AbilityId::ShedSkin, "Shed Skin", 3,
			12.0f, 0.0f, 0.35f,
			{ sf::Color(236, 232, 242), sf::Color(210, 205, 225), sf::Color(190, 180, 210),
			  sf::Color::Transparent, false, true, false, false }
		},
		{
			AbilityId::ShadowDecoy, "Shadow Decoy", 4,
			12.0f, 4.0f, 0.0f,
			{ sf::Color(46, 42, 70), sf::Color(28, 24, 48), sf::Color(92, 84, 138),
			  sf::Color::Transparent, false, true, true, true }
		},
		{
			AbilityId::TimeFreeze, "Time Freeze", 5,
			15.0f, 4.0f, 0.0f,
			{ sf::Color(180, 225, 255), sf::Color(110, 190, 255), sf::Color(150, 210, 255),
			  sf::Color::Transparent, false, false, true, false }
		},
		{
			AbilityId::VenomTrail, "Venom Trail", 6,
			10.0f, 3.0f, 0.0f,
			{ sf::Color(80, 235, 120), sf::Color(55, 185, 80), sf::Color(65, 215, 95),
			  sf::Color::Transparent, false, false, true, false }
		},
		{
			AbilityId::InkAnchor, "Ink Anchor", 7,
			12.0f, 5.0f, 0.0f,
			{ sf::Color(170, 68, 38), sf::Color(125, 42, 25), sf::Color(120, 38, 22),
			  sf::Color::Transparent, false, false, true, true }
		},
		{
			AbilityId::HuntersDash, "Hunter's Dash", 8,
			8.0f, 0.0f, 0.25f,
			{ sf::Color(120, 240, 255), sf::Color(80, 210, 245), sf::Color(115, 225, 255),
			  sf::Color::Transparent, false, false, true, false }
		},
		{
			AbilityId::InkMemory, "Ink Memory", 9,
			10.0f, 6.0f, 0.0f,
			{ sf::Color(205, 150, 255), sf::Color(150, 105, 225), sf::Color(170, 130, 240),
			  sf::Color::Transparent, false, false, true, false }
		}
	}};

	float ClampTimer(float l_value)
	{
		return (l_value > 0.0f) ? l_value : 0.0f;
	}
}

const std::array<AbilityDefinition, ABILITY_COUNT>& GetAllAbilityDefinitions()
{
	return kAbilityDefinitions;
}

const AbilityDefinition& GetAbilityDefinition(AbilityId l_id)
{
	if (l_id == AbilityId::None)
		return kNoneAbilityDefinition;

	return kAbilityDefinitions[GetAbilityIndex(l_id)];
}

bool IsValidAbilityId(AbilityId l_id, bool l_allowNone)
{
	switch (l_id)
	{
		case AbilityId::None:
			return l_allowNone;
		case AbilityId::InkFlare:
		case AbilityId::ShedSkin:
		case AbilityId::ShadowDecoy:
		case AbilityId::TimeFreeze:
		case AbilityId::VenomTrail:
		case AbilityId::InkAnchor:
		case AbilityId::HuntersDash:
		case AbilityId::InkMemory:
			return true;
		default:
			return false;
	}
}

std::size_t GetAbilityIndex(AbilityId l_id)
{
	switch (l_id)
	{
		case AbilityId::InkFlare: return 0;
		case AbilityId::ShedSkin: return 1;
		case AbilityId::ShadowDecoy: return 2;
		case AbilityId::TimeFreeze: return 3;
		case AbilityId::VenomTrail: return 4;
		case AbilityId::InkAnchor: return 5;
		case AbilityId::HuntersDash: return 6;
		case AbilityId::InkMemory: return 7;
		case AbilityId::None:
			return 0;
		default:
			assert(false && "GetAbilityIndex: unexpected AbilityId");
			return 0;
	}
}

AbilityId GetAbilityIdForIndex(std::size_t l_index)
{
	assert(l_index < ABILITY_COUNT);
	return kAbilityDefinitions[l_index].id;
}

AbilityId GetDefaultEquippedAbility()
{
	return AbilityId::InkFlare;
}

AbilityId ResolveEquippedAbilityFromUnlocks(const bool (&l_unlocked)[ABILITY_COUNT], AbilityId l_preferred)
{
	if (IsValidAbilityId(l_preferred, false) && l_unlocked[GetAbilityIndex(l_preferred)])
		return l_preferred;

	const AbilityId defaultId = GetDefaultEquippedAbility();
	if (l_unlocked[GetAbilityIndex(defaultId)])
		return defaultId;

	for (std::size_t i = 0; i < ABILITY_COUNT; ++i)
	{
		if (l_unlocked[i])
			return GetAbilityIdForIndex(i);
	}

	return AbilityId::None;
}

AbilityController::AbilityController()
	: m_equipped(GetDefaultEquippedAbility()),
	  m_active(AbilityId::None),
	  m_activeRemaining(0.0f),
	  m_visualRemaining(0.0f)
{
}

void AbilityController::ResetRuntime()
{
	for (AbilityState& state : m_states)
		state.cooldownRemaining = 0.0f;

	m_active = AbilityId::None;
	m_activeRemaining = 0.0f;
	m_visualRemaining = 0.0f;
}

void AbilityController::LoadPersistentProgress(const bool (&l_unlocked)[ABILITY_COUNT], AbilityId l_equipped)
{
	for (std::size_t i = 0; i < ABILITY_COUNT; ++i)
	{
		m_states[i].unlocked = l_unlocked[i];
		m_states[i].cooldownRemaining = 0.0f;
	}

	m_equipped = ResolveEquippedAbilityFromUnlocks(l_unlocked, l_equipped);

	m_active = AbilityId::None;
	m_activeRemaining = 0.0f;
	m_visualRemaining = 0.0f;
}

void AbilityController::ExportPersistentProgress(bool (&l_unlocked)[ABILITY_COUNT], AbilityId& l_equipped) const
{
	for (std::size_t i = 0; i < ABILITY_COUNT; ++i)
		l_unlocked[i] = m_states[i].unlocked;

	l_equipped = m_equipped;
}

bool AbilityController::Unlock(AbilityId l_id)
{
	if (l_id == AbilityId::None)
		return false;

	AbilityState& state = m_states[GetAbilityIndex(l_id)];
	bool wasUnlocked = state.unlocked;
	state.unlocked = true;
	return !wasUnlocked;
}

bool AbilityController::IsUnlocked(AbilityId l_id) const
{
	if (l_id == AbilityId::None)
		return false;

	return m_states[GetAbilityIndex(l_id)].unlocked;
}

bool AbilityController::SetEquipped(AbilityId l_id)
{
	if (l_id == AbilityId::None)
		return false;
	if (!IsUnlocked(l_id))
		return false;

	m_equipped = l_id;
	return true;
}

bool AbilityController::CycleEquipped(int l_direction)
{
	if (l_direction == 0)
		return false;

	std::array<AbilityId, ABILITY_COUNT> unlockedIds{};
	std::size_t unlockedCount = 0;
	for (std::size_t i = 0; i < ABILITY_COUNT; ++i)
	{
		if (m_states[i].unlocked)
			unlockedIds[unlockedCount++] = GetAbilityIdForIndex(i);
	}

	if (unlockedCount == 0)
		return false;

	std::size_t current = 0;
	bool foundEquipped = false;
	for (std::size_t i = 0; i < unlockedCount; ++i)
	{
		if (unlockedIds[i] == m_equipped)
		{
			current = i;
			foundEquipped = true;
			break;
		}
	}

	int step = (l_direction > 0) ? 1 : -1;
	if (!foundEquipped)
	{
		// m_equipped is None or not unlocked — advance from a consistent base so we
		// don't skip unlockedIds[0] (would happen if current stayed 0 and step == +1).
		if (step > 0)
			current = unlockedCount - 1;
		else
			current = 0;
	}

	int next = static_cast<int>(current) + step;
	if (next < 0)
		next = static_cast<int>(unlockedCount) - 1;
	else if (next >= static_cast<int>(unlockedCount))
		next = 0;

	m_equipped = unlockedIds[static_cast<std::size_t>(next)];
	return true;
}

bool AbilityController::TryActivateEquipped()
{
	if (m_equipped == AbilityId::None)
		return false;
	if (!IsUnlocked(m_equipped))
		return false;
	if (m_active != AbilityId::None)
		return false;

	AbilityState& state = m_states[GetAbilityIndex(m_equipped)];
	if (state.cooldownRemaining > 0.0f)
		return false;

	const AbilityDefinition& def = GetEquippedDefinition();
	state.cooldownRemaining = def.cooldownSec;
	m_active = m_equipped;
	m_activeRemaining = def.durationSec;
	m_visualRemaining = def.GetVisualDuration();
	return true;
}

void AbilityController::Update(float l_dt)
{
	for (AbilityState& state : m_states)
		state.cooldownRemaining = ClampTimer(state.cooldownRemaining - l_dt);

	if (m_active == AbilityId::None)
		return;

	if (m_activeRemaining > 0.0f)
		m_activeRemaining = ClampTimer(m_activeRemaining - l_dt);

	if (m_visualRemaining > 0.0f)
		m_visualRemaining = ClampTimer(m_visualRemaining - l_dt);

	if (m_activeRemaining <= 0.0f && m_visualRemaining <= 0.0f)
	{
		m_active = AbilityId::None;
		m_activeRemaining = 0.0f;
		m_visualRemaining = 0.0f;
	}
}

float AbilityController::GetCooldownRemaining(AbilityId l_id) const
{
	if (l_id == AbilityId::None)
		return 0.0f;

	return m_states[GetAbilityIndex(l_id)].cooldownRemaining;
}

AbilityStatus AbilityController::GetStatus(AbilityId l_id) const
{
	if (l_id == AbilityId::None)
		return AbilityStatus::Locked;
	if (!IsUnlocked(l_id))
		return AbilityStatus::Locked;
	if (m_active == l_id)
	{
		if (m_activeRemaining > 0.0f)
			return AbilityStatus::Active;
		if (m_visualRemaining > 0.0f)
			return AbilityStatus::VisualOnly;
	}
	if (GetCooldownRemaining(l_id) > 0.0f)
		return AbilityStatus::Cooldown;
	return AbilityStatus::Ready;
}

const AbilityDefinition& AbilityController::GetEquippedDefinition() const
{
	return GetAbilityDefinition(m_equipped);
}

const AbilityDefinition* AbilityController::GetVisualDefinition() const
{
	if (m_active == AbilityId::None || m_visualRemaining <= 0.0f)
		return nullptr;

	return &GetAbilityDefinition(m_active);
}
