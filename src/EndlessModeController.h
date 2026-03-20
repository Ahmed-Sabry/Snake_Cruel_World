#pragma once

#include <vector>
#include <string>

struct MechanicChangeEvent
{
	bool changed = false;
	int activatedMechanic = -1;
	int deactivatedMechanic = -1;
	std::string warningText;
};

class EndlessModeController
{
public:
	EndlessModeController(int l_highestUnlockedLevel);

	MechanicChangeEvent Update(float l_dt);

	float GetSurvivalTime() const { return m_survivalTime; }
	float GetSpeedMultiplier() const { return m_speedMultiplier; }
	float GetCorruption() const;
	bool IsMechanicActive(int l_mechanicId) const;

	// Mechanic IDs
	static constexpr int MECH_BLACKOUTS       = 0;
	static constexpr int MECH_QUICKSAND       = 1;
	static constexpr int MECH_MIRROR_GHOST    = 2;
	static constexpr int MECH_TIMED_APPLES    = 3;
	static constexpr int MECH_POISON_APPLES   = 4;
	static constexpr int MECH_EARTHQUAKES     = 5;
	static constexpr int MECH_PREDATOR        = 6;
	static constexpr int MECH_CONTROL_SHUFFLE = 7;
	static constexpr int NUM_MECHANICS        = 8;

private:
	static const char* s_mechanicNames[NUM_MECHANICS];

	float m_survivalTime;
	int m_cycleIndex;
	float m_cycleTimer;
	float m_cycleDuration;
	float m_speedMultiplier;
	int m_maxConcurrent;

	std::vector<int> m_availableMechanics;
	struct ActiveMechanic { int mechanicId; };
	std::vector<ActiveMechanic> m_activeMechanics;

	float m_warningTimer;
	int m_pendingMechanic;
	bool m_warningActive;
};
