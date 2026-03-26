#pragma once

#include "AchievementDefs.h"
#include "StatsManager.h"
#include <queue>

class AudioManager;

// Snapshot of game state passed to achievement checks
struct AchievementContext
{
	int levelId = 0;
	int score = 0;
	float levelTime = 0.0f;
	int selfCollisions = 0;
	int consecutiveApples = 0;
	float comboMultiplier = 1.0f;
	int applesEaten = 0;
	int applesToWin = 0;
	int predatorApplesEaten = 0;
	int snakeBodySize = 0;
	bool blackoutActive = false;
	bool screenFlipped = false;
	float screenFlipStartTime = 0.0f;
	int quicksandTouches = 0;
	int timedAppleMisses = 0;
	int poisonApplesThisLevel = 0;
	bool reachedMinBodyFromCollision = false;
	const CumulativeStats* stats = nullptr;
	const int* starRatings = nullptr;
	int completedLevelCount = 0;
	int totalDeaths = 0;
	float predatorDistToApple = 999.0f; // distance of predator to apple when player eats it
};

class AchievementManager
{
public:
	AchievementManager();

	// Notification methods — called from PlayState hooks
	void OnAppleEaten(const AchievementContext& l_ctx);
	void OnDeath(const AchievementContext& l_ctx);
	void OnLevelComplete(const AchievementContext& l_ctx);
	void OnSelfCollision(const AchievementContext& l_ctx);
	void OnPoisonAppleEaten(const AchievementContext& l_ctx);
	void OnStatsUpdate(const AchievementContext& l_ctx);
	void OnKonamiCode();

	// Query
	bool IsUnlocked(AchievementId l_id) const;
	int GetUnlockedCount() const;

	// Notification queue for in-game popups
	bool HasPendingNotification() const;
	AchievementId PopNotification();

	// Persistence
	const bool* GetUnlockData() const { return m_unlocked; }
	void SetUnlockData(const bool* l_data);

	// Audio reference for unlock sound
	void SetAudioManager(AudioManager* l_audio) { m_audio = l_audio; }

private:
	void Unlock(AchievementId l_id);

	bool m_unlocked[NUM_ACHIEVEMENTS] = {};
	std::queue<AchievementId> m_pendingNotifications;
	AudioManager* m_audio = nullptr;
};
