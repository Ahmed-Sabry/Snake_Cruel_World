#include "EndlessModeController.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

const char* EndlessModeController::s_mechanicNames[NUM_MECHANICS] = {
	"Blackouts",
	"Quicksand",
	"Mirror Ghost",
	"Timed Apples",
	"Poison Apples",
	"Earthquakes",
	"Predator",
	"Control Shuffle"
};

EndlessModeController::EndlessModeController(int l_highestUnlockedLevel)
	: m_survivalTime(0.0f),
	  m_cycleIndex(0),
	  m_cycleTimer(0.0f),
	  m_cycleDuration(35.0f),
	  m_speedMultiplier(1.0f),
	  m_maxConcurrent(1),
	  m_warningTimer(0.0f),
	  m_pendingMechanic(-1),
	  m_warningActive(false)
{
	// Build available mechanics from beaten levels
	// Mechanic i available if player has beaten level i+2
	// (Level 2 = Blackouts, Level 3 = Quicksand, etc.)
	for (int i = 0; i < NUM_MECHANICS; i++)
	{
		int requiredLevel = i + 2; // Level that introduces this mechanic
		if (l_highestUnlockedLevel > requiredLevel)
			m_availableMechanics.push_back(i);
	}

	// Guarantee at least blackouts + quicksand if level 5+ beaten
	if (m_availableMechanics.empty())
		m_availableMechanics.push_back(MECH_BLACKOUTS);
}

MechanicChangeEvent EndlessModeController::Update(float l_dt)
{
	MechanicChangeEvent event;
	m_survivalTime += l_dt;

	// Gradual speed ramp (capped at 2.0x)
	m_speedMultiplier = std::min(2.0f, 1.0f + m_survivalTime * 0.008f);

	// Difficulty thresholds
	if (m_survivalTime >= 180.0f)
	{
		m_maxConcurrent = 3;
		m_cycleDuration = 25.0f;
	}
	else if (m_survivalTime >= 90.0f)
	{
		m_maxConcurrent = 2;
		m_cycleDuration = 30.0f;
	}

	// Warning phase: show warning 2 seconds before activating
	if (m_warningActive)
	{
		m_warningTimer -= l_dt;
		if (m_warningTimer <= 0.0f)
		{
			m_warningActive = false;

			// Actually activate the mechanic
			if (m_pendingMechanic >= 0)
			{
				// Remove oldest if at max
				int deactivated = -1;
				if ((int)m_activeMechanics.size() >= m_maxConcurrent && !m_activeMechanics.empty())
				{
					deactivated = m_activeMechanics.front().mechanicId;
					m_activeMechanics.erase(m_activeMechanics.begin());
				}

				m_activeMechanics.push_back({ m_pendingMechanic });
				event.changed = true;
				event.activatedMechanic = m_pendingMechanic;
				event.deactivatedMechanic = deactivated;
				event.warningText = "";
				m_pendingMechanic = -1;
			}
		}
		return event;
	}

	// Cycle timer
	m_cycleTimer += l_dt;
	if (m_cycleTimer >= m_cycleDuration && !m_availableMechanics.empty())
	{
		m_cycleTimer = 0.0f;
		m_cycleIndex++;

		// Pick a random mechanic not already active
		std::vector<int> candidates;
		for (int m : m_availableMechanics)
		{
			bool alreadyActive = false;
			for (const auto& am : m_activeMechanics)
			{
				if (am.mechanicId == m) { alreadyActive = true; break; }
			}
			if (!alreadyActive)
				candidates.push_back(m);
		}

		if (!candidates.empty())
		{
			int chosen = candidates[std::rand() % candidates.size()];
			m_pendingMechanic = chosen;
			m_warningActive = true;
			m_warningTimer = 2.0f;

			event.warningText = std::string("Incoming: ") + s_mechanicNames[chosen];
		}
	}

	return event;
}

float EndlessModeController::GetCorruption() const
{
	return std::min(1.0f, m_survivalTime / 300.0f);
}

bool EndlessModeController::IsMechanicActive(int l_mechanicId) const
{
	for (const auto& am : m_activeMechanics)
	{
		if (am.mechanicId == l_mechanicId) return true;
	}
	return false;
}
