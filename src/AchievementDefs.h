#pragma once

#include <vector>

constexpr int NUM_ACHIEVEMENTS = 30;

enum class AchievementId
{
	// Skill (8)
	Flawless = 0,
	SpeedDemon,
	ApexPredator,
	Perfectionist,
	ComboKing,
	NoHesitation,
	Marathon,
	IronSnake,
	// Cruelty (8)
	WelcomeCruelWorld,
	WallHugger,
	SelfDestructive,
	Deceived,
	LunchThief,
	SnakeSnack,
	Appetizer,
	Glutton,
	// Persistence (7)
	Stubborn,
	Completionist,
	CruelMaster,
	Dedicated,
	Centurion,
	AppleFarmer,
	EndlessSurvivor,
	// Hidden (7)
	FloorIsLava,
	BlindFaith,
	PlotTwist,
	Ouroboros,
	AppleThief,
	AgainstAllOdds,
	KonamiMaster
};

enum class AchievementCategory
{
	Skill,
	Cruelty,
	Persistence,
	Hidden
};

struct AchievementDef
{
	AchievementId id;
	const char* name;
	const char* description;
	AchievementCategory category;
	bool hidden;
};

inline std::vector<AchievementDef> GetAllAchievements()
{
	return {
		// === Skill (8) ===
		{ AchievementId::Flawless, "Flawless",
		  "Complete any level with 0 self-collisions",
		  AchievementCategory::Skill, false },
		{ AchievementId::SpeedDemon, "Speed Demon",
		  "Complete any level in under 60 seconds",
		  AchievementCategory::Skill, false },
		{ AchievementId::ApexPredator, "Apex Predator",
		  "Beat Level 8 before the predator eats any apple",
		  AchievementCategory::Skill, false },
		{ AchievementId::Perfectionist, "Perfectionist",
		  "Earn 3 stars on all 10 levels",
		  AchievementCategory::Skill, false },
		{ AchievementId::ComboKing, "Combo King",
		  "Reach a 3x combo multiplier",
		  AchievementCategory::Skill, false },
		{ AchievementId::NoHesitation, "No Hesitation",
		  "Eat every timed apple in Level 5 without missing any",
		  AchievementCategory::Skill, false },
		{ AchievementId::Marathon, "Marathon",
		  "Score 5,000+ points in a single level",
		  AchievementCategory::Skill, false },
		{ AchievementId::IronSnake, "Iron Snake",
		  "Complete Level 10 with fewer than 3 self-collisions",
		  AchievementCategory::Skill, false },

		// === Cruelty (8) ===
		{ AchievementId::WelcomeCruelWorld, "Welcome to the Cruel World",
		  "Die for the first time",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::WallHugger, "Wall Hugger",
		  "Die 50 times total",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::SelfDestructive, "Self-Destructive",
		  "Lose 100 total segments to self-collision",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::Deceived, "Deceived",
		  "Eat 10 poison apples total",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::LunchThief, "Lunch Thief",
		  "Have the predator steal 10 of your apples",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::SnakeSnack, "Snake Snack",
		  "Get killed by the predator 5 times",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::Appetizer, "Appetizer",
		  "Die in under 3 seconds",
		  AchievementCategory::Cruelty, false },
		{ AchievementId::Glutton, "Glutton",
		  "Eat 3 poison apples in a single level",
		  AchievementCategory::Cruelty, false },

		// === Persistence (7) ===
		{ AchievementId::Stubborn, "Stubborn",
		  "Retry a single level 25 times",
		  AchievementCategory::Persistence, false },
		{ AchievementId::Completionist, "Completionist",
		  "Beat all 10 levels",
		  AchievementCategory::Persistence, false },
		{ AchievementId::CruelMaster, "Cruel Master",
		  "Earn 3 stars on Level 10",
		  AchievementCategory::Persistence, false },
		{ AchievementId::Dedicated, "Dedicated",
		  "Play for a total of 1 hour",
		  AchievementCategory::Persistence, false },
		{ AchievementId::Centurion, "Centurion",
		  "Die 100 times total",
		  AchievementCategory::Persistence, false },
		{ AchievementId::AppleFarmer, "Apple Farmer",
		  "Eat 500 apples total",
		  AchievementCategory::Persistence, false },
		{ AchievementId::EndlessSurvivor, "Endless Survivor",
		  "Survive 120 seconds in Endless Mode",
		  AchievementCategory::Persistence, false },

		// === Hidden (7) ===
		{ AchievementId::FloorIsLava, "The Floor is Lava",
		  "Complete Level 3 without touching quicksand",
		  AchievementCategory::Hidden, true },
		{ AchievementId::BlindFaith, "Blind Faith",
		  "Eat the final apple during a blackout in Level 2",
		  AchievementCategory::Hidden, true },
		{ AchievementId::PlotTwist, "Plot Twist",
		  "Complete the upside-down section of Level 10 in under 10 seconds",
		  AchievementCategory::Hidden, true },
		{ AchievementId::Ouroboros, "Ouroboros",
		  "Cut yourself to exactly 1 segment, then finish the level",
		  AchievementCategory::Hidden, true },
		{ AchievementId::AppleThief, "Apple Thief",
		  "Steal an apple the predator was about to eat",
		  AchievementCategory::Hidden, true },
		{ AchievementId::AgainstAllOdds, "Against All Odds",
		  "Complete Level 10 on your first ever attempt",
		  AchievementCategory::Hidden, true },
		{ AchievementId::KonamiMaster, "Secret Menu",
		  "Use the Konami code in Level Select",
		  AchievementCategory::Hidden, true },
	};
}
