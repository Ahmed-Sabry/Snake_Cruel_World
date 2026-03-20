#include "StatsManager.h"
#include <algorithm>

StatsManager::StatsManager()
	: m_playtimeAccumulator(0.0f)
{
}

void StatsManager::OnAppleEaten()
{
	m_stats.totalApplesEaten++;
}

void StatsManager::OnSelfCollision(int l_segmentsLost)
{
	m_stats.totalSegmentsLost += l_segmentsLost;
}

void StatsManager::OnPoisonAppleEaten()
{
	m_stats.totalPoisonApplesEaten++;
}

void StatsManager::OnDeath(int l_levelId)
{
	int idx = l_levelId - 1;
	if (idx >= 0 && idx < NUM_LEVELS)
		m_stats.deathsPerLevel[idx]++;
}

void StatsManager::OnLevelStart(int l_levelId)
{
	int idx = l_levelId - 1;
	if (idx >= 0 && idx < NUM_LEVELS)
		m_stats.attemptsPerLevel[idx]++;
}

void StatsManager::OnLevelComplete(int l_levelId, float l_time, int l_score)
{
	int idx = l_levelId - 1;
	if (idx >= 0 && idx < NUM_LEVELS && l_time > 0.0f)
	{
		if (m_stats.fastestLevelTimes[idx] <= 0.0f || l_time < m_stats.fastestLevelTimes[idx])
			m_stats.fastestLevelTimes[idx] = l_time;
	}
	if (l_score > m_stats.bestSingleLevelScore)
		m_stats.bestSingleLevelScore = l_score;
}

void StatsManager::OnComboAchieved(int l_consecutive)
{
	if (l_consecutive > m_stats.bestCombo)
		m_stats.bestCombo = l_consecutive;
}

void StatsManager::UpdatePlaytime(float l_dt)
{
	m_playtimeAccumulator += l_dt;
	while (m_playtimeAccumulator >= 1.0f)
	{
		m_stats.totalPlaytimeSeconds++;
		m_playtimeAccumulator -= 1.0f;
	}
}

void StatsManager::OnPredatorAteApple()
{
	m_stats.predatorApplesStolen++;
}

void StatsManager::OnPredatorKilledPlayer()
{
	m_stats.predatorKills++;
}

void StatsManager::OnEndlessGameOver(int l_score, float l_time)
{
	if (l_score > m_stats.endlessBestScore)
		m_stats.endlessBestScore = l_score;
	if (l_time > m_stats.endlessBestTime)
		m_stats.endlessBestTime = l_time;
}
