#pragma once

#include "LevelConfig.h"

struct CumulativeStats
{
	int totalApplesEaten = 0;
	int totalSegmentsLost = 0;
	int totalPoisonApplesEaten = 0;
	int totalPlaytimeSeconds = 0;
	int bestCombo = 0;
	int bestSingleLevelScore = 0;
	float fastestLevelTimes[NUM_LEVELS] = {};
	int deathsPerLevel[NUM_LEVELS] = {};
	int attemptsPerLevel[NUM_LEVELS] = {};
	int endlessBestScore = 0;
	float endlessBestTime = 0.0f;
	int predatorApplesStolen = 0;
	int predatorKills = 0;
};

class StatsManager
{
public:
	StatsManager();

	// Notification methods — called from PlayState hooks
	void OnAppleEaten();
	void OnSelfCollision(int l_segmentsLost);
	void OnPoisonAppleEaten();
	void OnDeath(int l_levelId);
	void OnLevelStart(int l_levelId);
	void OnLevelComplete(int l_levelId, float l_time, int l_score);
	void OnComboAchieved(int l_consecutive);
	void UpdatePlaytime(float l_dt);
	void OnPredatorAteApple();
	void OnPredatorKilledPlayer();
	void OnEndlessGameOver(int l_score, float l_time);

	const CumulativeStats& GetStats() const { return m_stats; }
	CumulativeStats& GetMutableStats() { return m_stats; }

private:
	CumulativeStats m_stats;
	float m_playtimeAccumulator; // sub-second accumulator
};
