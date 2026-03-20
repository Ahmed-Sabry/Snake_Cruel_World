#include "AchievementManager.h"
#include "AudioManager.h"
#include <cstring>

AchievementManager::AchievementManager()
{
	std::memset(m_unlocked, 0, sizeof(m_unlocked));
}

void AchievementManager::OnAppleEaten(const AchievementContext& l_ctx)
{
	// ComboKing: reach 3x combo multiplier
	if (l_ctx.comboMultiplier >= 3.0f)
		Unlock(AchievementId::ComboKing);

	// BlindFaith: eat final apple during blackout in Level 2
	if (l_ctx.levelId == 2 && l_ctx.blackoutActive &&
		l_ctx.applesEaten >= l_ctx.applesToWin)
		Unlock(AchievementId::BlindFaith);

	// AppleThief: steal apple predator was about to eat (within 3 tiles)
	if (l_ctx.predatorDistToApple <= 3.0f)
		Unlock(AchievementId::AppleThief);
}

void AchievementManager::OnDeath(const AchievementContext& l_ctx)
{
	int totalDeaths = l_ctx.totalDeaths;

	// WelcomeCruelWorld: die for the first time
	if (totalDeaths >= 1)
		Unlock(AchievementId::WelcomeCruelWorld);

	// WallHugger: die 50 times total
	if (totalDeaths >= 50)
		Unlock(AchievementId::WallHugger);

	// Centurion: die 100 times total
	if (totalDeaths >= 100)
		Unlock(AchievementId::Centurion);

	// Appetizer: die in under 3 seconds
	if (l_ctx.levelTime < 3.0f)
		Unlock(AchievementId::Appetizer);

	// SnakeSnack: predator kills (tracked via stats)
	if (l_ctx.stats && l_ctx.stats->predatorKills >= 5)
		Unlock(AchievementId::SnakeSnack);
}

void AchievementManager::OnLevelComplete(const AchievementContext& l_ctx)
{
	// Flawless: complete any level with 0 self-collisions
	if (l_ctx.selfCollisions == 0)
		Unlock(AchievementId::Flawless);

	// SpeedDemon: complete any level in under 60 seconds
	if (l_ctx.levelTime < 60.0f)
		Unlock(AchievementId::SpeedDemon);

	// ApexPredator: beat Level 8 before predator eats any apple
	if (l_ctx.levelId == 8 && l_ctx.predatorApplesEaten == 0)
		Unlock(AchievementId::ApexPredator);

	// NoHesitation: eat every timed apple in Level 5 without missing any
	if (l_ctx.levelId == 5 && l_ctx.timedAppleMisses == 0)
		Unlock(AchievementId::NoHesitation);

	// Marathon: score 5000+ in a single level
	if (l_ctx.score >= 5000)
		Unlock(AchievementId::Marathon);

	// IronSnake: complete Level 10 with fewer than 3 self-collisions
	if (l_ctx.levelId == 10 && l_ctx.selfCollisions < 3)
		Unlock(AchievementId::IronSnake);

	// FloorIsLava: complete Level 3 without touching quicksand
	if (l_ctx.levelId == 3 && l_ctx.quicksandTouches == 0)
		Unlock(AchievementId::FloorIsLava);

	// PlotTwist: complete upside-down section of Level 10 in under 10 seconds
	if (l_ctx.levelId == 10 && l_ctx.screenFlipped &&
		l_ctx.screenFlipStartTime > 0.0f &&
		(l_ctx.levelTime - l_ctx.screenFlipStartTime) < 10.0f)
		Unlock(AchievementId::PlotTwist);

	// Ouroboros: cut to 1 segment via self-collision, then finish
	if (l_ctx.reachedMinBodyFromCollision)
		Unlock(AchievementId::Ouroboros);

	// AgainstAllOdds: complete Level 10 on first attempt
	if (l_ctx.levelId == 10 && l_ctx.stats &&
		l_ctx.stats->attemptsPerLevel[9] == 1)
		Unlock(AchievementId::AgainstAllOdds);

	// Completionist: beat all 10 levels
	// highestUnlockedLevel reaches NUM_LEVELS when the last level is beaten
	if (l_ctx.highestUnlockedLevel >= NUM_LEVELS)
		Unlock(AchievementId::Completionist);

	// Perfectionist: 3 stars on all 10 levels
	if (l_ctx.starRatings)
	{
		bool allThree = true;
		for (int i = 0; i < NUM_LEVELS; i++)
		{
			if (l_ctx.starRatings[i] < 3) { allThree = false; break; }
		}
		if (allThree) Unlock(AchievementId::Perfectionist);
	}

	// CruelMaster: 3 stars on Level 10
	if (l_ctx.starRatings && l_ctx.starRatings[9] >= 3)
		Unlock(AchievementId::CruelMaster);
}

void AchievementManager::OnSelfCollision(const AchievementContext& l_ctx)
{
	// SelfDestructive: lose 100 total segments
	if (l_ctx.stats && l_ctx.stats->totalSegmentsLost >= 100)
		Unlock(AchievementId::SelfDestructive);
}

void AchievementManager::OnPoisonAppleEaten(const AchievementContext& l_ctx)
{
	// Deceived: eat 10 poison apples total
	if (l_ctx.stats && l_ctx.stats->totalPoisonApplesEaten >= 10)
		Unlock(AchievementId::Deceived);

	// Glutton: eat 3 poison apples in a single level
	if (l_ctx.poisonApplesThisLevel >= 3)
		Unlock(AchievementId::Glutton);
}

void AchievementManager::OnStatsUpdate(const AchievementContext& l_ctx)
{
	if (!l_ctx.stats) return;

	const auto& stats = *l_ctx.stats;

	// Stubborn: retry a single level 25 times
	for (int i = 0; i < NUM_LEVELS; i++)
	{
		if (stats.attemptsPerLevel[i] >= 25)
		{
			Unlock(AchievementId::Stubborn);
			break;
		}
	}

	// Dedicated: play for 1 hour
	if (stats.totalPlaytimeSeconds >= 3600)
		Unlock(AchievementId::Dedicated);

	// AppleFarmer: eat 500 apples total
	if (stats.totalApplesEaten >= 500)
		Unlock(AchievementId::AppleFarmer);

	// LunchThief: predator steals 10 apples
	if (stats.predatorApplesStolen >= 10)
		Unlock(AchievementId::LunchThief);

	// EndlessSurvivor: survive 120 seconds in endless mode
	if (stats.endlessBestTime >= 120.0f)
		Unlock(AchievementId::EndlessSurvivor);
}

void AchievementManager::OnKonamiCode()
{
	Unlock(AchievementId::KonamiMaster);
}

bool AchievementManager::IsUnlocked(AchievementId l_id) const
{
	int idx = static_cast<int>(l_id);
	if (idx < 0 || idx >= NUM_ACHIEVEMENTS) return false;
	return m_unlocked[idx];
}

int AchievementManager::GetUnlockedCount() const
{
	int count = 0;
	for (int i = 0; i < NUM_ACHIEVEMENTS; i++)
	{
		if (m_unlocked[i]) count++;
	}
	return count;
}

bool AchievementManager::HasPendingNotification() const
{
	return !m_pendingNotifications.empty();
}

AchievementId AchievementManager::PopNotification()
{
	if (m_pendingNotifications.empty())
		return AchievementId::Flawless; // fallback — callers must check HasPendingNotification() first

	AchievementId id = m_pendingNotifications.front();
	m_pendingNotifications.pop();
	return id;
}

void AchievementManager::SetUnlockData(const bool* l_data)
{
	if (!l_data) return;
	std::memcpy(m_unlocked, l_data, sizeof(m_unlocked));
}

void AchievementManager::Unlock(AchievementId l_id)
{
	int idx = static_cast<int>(l_id);
	if (idx < 0 || idx >= NUM_ACHIEVEMENTS) return;
	if (m_unlocked[idx]) return; // already unlocked

	m_unlocked[idx] = true;
	m_pendingNotifications.push(l_id);

	if (m_audio)
		m_audio->PlaySound("achievement_unlock");
}
