#pragma once

#include <SFML/Graphics/Color.hpp>
#include <array>
#include <cstddef>

enum class AbilityId
{
	None = 0,
	InkFlare,
	ShedSkin,
	ShadowDecoy,
	TimeFreeze,
	VenomTrail,
	InkAnchor,
	HuntersDash,
	InkMemory
};

inline constexpr std::size_t ABILITY_COUNT = 8;

enum class AbilityStatus
{
	Locked,
	Ready,
	Active,
	VisualOnly,
	Cooldown
};

struct AbilityVisualSpec
{
	sf::Color headColor = sf::Color::White;
	sf::Color bodyColor = sf::Color::White;
	sf::Color inkTint = sf::Color(60, 50, 45);
	sf::Color gradientEnd = sf::Color::Transparent;
	bool rainbow = false;
	bool translucent = false;
	bool thickOutline = false;
	bool invertEyes = false;
};

struct AbilityDefinition
{
	AbilityId id = AbilityId::None;
	const char* name = "";
	int unlockLevelId = 0;
	float cooldownSec = 0.0f;
	float durationSec = 0.0f;
	float instantVisualSec = 0.0f;
	AbilityVisualSpec visual;

	bool IsInstant() const
	{
		return durationSec <= 0.0f;
	}

	float GetVisualDuration() const
	{
		return IsInstant() ? instantVisualSec : durationSec;
	}
};

struct AbilityState
{
	bool unlocked = false;
	float cooldownRemaining = 0.0f;
};

const std::array<AbilityDefinition, ABILITY_COUNT>& GetAllAbilityDefinitions();
const AbilityDefinition& GetAbilityDefinition(AbilityId l_id);
bool IsValidAbilityId(AbilityId l_id, bool l_allowNone = true);
std::size_t GetAbilityIndex(AbilityId l_id);
AbilityId GetAbilityIdForIndex(std::size_t l_index);
AbilityId GetDefaultEquippedAbility();
AbilityId ResolveEquippedAbilityFromUnlocks(const bool (&l_unlocked)[ABILITY_COUNT], AbilityId l_preferred);

class AbilityController
{
public:
	AbilityController();

	void ResetRuntime();
	void LoadPersistentProgress(const bool (&l_unlocked)[ABILITY_COUNT], AbilityId l_equipped);
	void ExportPersistentProgress(bool (&l_unlocked)[ABILITY_COUNT], AbilityId& l_equipped) const;

	bool Unlock(AbilityId l_id);
	bool IsUnlocked(AbilityId l_id) const;
	bool SetEquipped(AbilityId l_id);
	bool CycleEquipped(int l_direction);
	bool TryActivateEquipped();
	void Update(float l_dt);

	AbilityId GetEquipped() const { return m_equipped; }
	AbilityId GetActive() const { return m_active; }
	float GetActiveRemaining() const { return m_activeRemaining; }
	float GetVisualRemaining() const { return m_visualRemaining; }
	float GetCooldownRemaining(AbilityId l_id) const;
	AbilityStatus GetStatus(AbilityId l_id) const;

	const AbilityDefinition& GetEquippedDefinition() const;
	const AbilityDefinition* GetVisualDefinition() const;

private:
	std::array<AbilityState, ABILITY_COUNT> m_states;
	AbilityId m_equipped;
	AbilityId m_active;
	float m_activeRemaining;
	float m_visualRemaining;
};
