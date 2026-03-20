#include "GameOverState.h"
#include "AudioManager.h"
#include "LevelConfig.h"
#include "SaveManager.h"
#include "StatsManager.h"
#include "AchievementManager.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <vector>

const char* GameOverState::s_deathTaunts[] = {
	"That was almost impressive.",
	"The walls send their regards.",
	"You are your own worst enemy. Literally.",
	"The call is coming from inside the snake.",
	"Better luck next time. Or not.",
	"The world barely had to try.",
	"Have you considered a different hobby?",
	"The snake deserved better.",
	"So close. And yet so far.",
	"Error 404: Skill not found.",
	"The game thanks you for the entertainment.",
	"That was educational. For us, not you.",
	"Gravity works differently when you're a snake.",
	"The walls barely noticed you.",
	"Achievement unlocked: Spectacular Failure.",
	"The cruel world yawns.",
	"That wasn't even your final form. Right?",
	"Somewhere, a wall just smiled.",
	"The snake had dreams. You had other plans.",
	"At least you were consistent."
};
const int GameOverState::s_deathTauntCount = 20;

const char* GameOverState::s_victoryTaunts[] = {
	"Impressive. The world will try harder next time.",
	"You survived. The world is disappointed.",
	"Don't let this go to your head.",
	"Victory. For now.",
	"The next level heard about this. It's angry.",
	"You've earned a brief moment of peace.",
	"Well done. Now do it with three stars.",
	"The cruel world grudgingly respects you."
};
const int GameOverState::s_victoryTauntCount = 8;

const char* GameOverState::s_fastDeathTaunts[] = {
	"That was... efficient.",
	"Speed run?",
	"Blink and you missed it. So did we.",
	"New personal worst!",
	"That might be a record. Not a good one.",
	"The loading screen lasted longer.",
	"Were you even trying?"
};
const int GameOverState::s_fastDeathTauntCount = 7;

// --- Context-sensitive taunt pools ---

static const char* s_firstDeathTaunts[] = {
	"Welcome to the Cruel World."
};

static const char* s_zeroAppleTaunts[] = {
	"Zero apples. Zero mercy.",
	"You didn't even eat one.",
	"The apple was right there."
};

static const char* s_almostWonTaunts[] = {
	"So close. That's the cruelest part.",
	"One more apple. Just one more.",
	"The world waited for this exact moment."
};

static const char* s_predatorKillTaunts[] = {
	"The Watcher sends its regards.",
	"It learned faster than you did.",
	"You looked delicious, apparently.",
	"The hunter became the hunted."
};

static const char* s_mirrorKillTaunts[] = {
	"Killed by yourself. Poetic.",
	"Your reflection was the better player.",
	"The enemy was you. Literally."
};

static const char* s_blackoutDeathTaunts[] = {
	"See? Neither could you.",
	"The dark isn't empty. You found out.",
	"Lights out. Permanently."
};

static const char* s_quicksandDeathTaunts[] = {
	"Stuck? What a shame.",
	"The floor won.",
	"Slow and steady loses the race."
};

static const char* s_highComboLostTaunts[] = {
	"apple combo. Gone. Just like that.",
	"in a row and then... that.",
	"apples without a mistake. Until the mistake."
};

static const char* s_highRetryTaunts[] = {
	"We admire the stubbornness.",
	"Still here? ...Respect.",
	"The walls have a betting pool on you.",
	"At this point, we're rooting for you. Quietly."
};

static const char* s_moderateRetryTaunts[] = {
	"The definition of insanity.",
	"The walls remember you.",
	"The world has seen this movie before."
};

static const char* s_gettingWorseTaunts[] = {
	"That was worse. Much worse.",
	"The game isn't getting harder. You're getting tired.",
	"Maybe take a break? ...Just kidding."
};

static const char* s_highDeathCountTaunts[] = {
	"We stopped feeling bad around #50.",
	"At this point, the snake is used to it.",
	"The walls have lost count. We haven't."
};

std::string GameOverState::SelectDeathTaunt(int l_levelId, int l_applesToWin)
{
	auto& sm = m_stateManager;

	// --- Priority-ordered evaluation (most specific first) ---

	// First ever death (highest priority — one-time moment)
	if (sm.totalDeaths == 1)
		return s_firstDeathTaunts[0];

	// Fast death (<3 seconds)
	if (sm.levelTime < 3.0f)
		return s_fastDeathTaunts[rand() % s_fastDeathTauntCount];

	// Zero apples eaten
	if (sm.deathAppleCount == 0 && sm.levelTime >= 3.0f)
		return s_zeroAppleTaunts[rand() % 3];

	// Almost won (within 2 apples of target)
	if (l_applesToWin > 0 && sm.deathAppleCount >= l_applesToWin - 2)
		return s_almostWonTaunts[rand() % 3];

	// Predator kill
	if (sm.deathCause == StateManager::DeathCause::Predator)
		return s_predatorKillTaunts[rand() % 4];

	// Mirror ghost kill
	if (sm.deathCause == StateManager::DeathCause::MirrorGhost)
		return s_mirrorKillTaunts[rand() % 3];

	// Died during blackout
	if (sm.wasInBlackout)
		return s_blackoutDeathTaunts[rand() % 3];

	// Died on quicksand
	if (sm.wasOnQuicksand)
		return s_quicksandDeathTaunts[rand() % 3];

	// Lost a high combo (4+ consecutive apples)
	if (sm.hadHighCombo && sm.comboLostAt >= 4)
	{
		int pick = rand() % 3;
		std::string prefix = std::to_string(sm.comboLostAt) + " ";
		return prefix + s_highComboLostTaunts[pick];
	}

	// High retry count (10+)
	if (sm.retryCount >= 10)
	{
		std::string prefix = "Attempt " + std::to_string(sm.retryCount + 1) + ". ";
		return prefix + s_highRetryTaunts[rand() % 4];
	}

	// Moderate retry count (5+)
	if (sm.retryCount >= 5)
	{
		std::string prefix = "Attempt " + std::to_string(sm.retryCount + 1) + ". ";
		return prefix + s_moderateRetryTaunts[rand() % 3];
	}

	// Getting worse (significantly fewer apples than session best, after 3+ retries)
	if (sm.retryCount >= 3 && sm.sessionBestApples > 3 &&
		sm.deathAppleCount < sm.sessionBestApples / 2)
		return s_gettingWorseTaunts[rand() % 3];

	// Improvement detected (within 80% of session best and improving)
	if (sm.retryCount >= 2 && sm.deathAppleCount == sm.sessionBestApples &&
		sm.deathAppleCount > 3)
		return "New session best. Getting closer.";

	// High total death count (100+)
	if (sm.totalDeaths >= 100)
	{
		std::string prefix = "Death #" + std::to_string(sm.totalDeaths) + ". ";
		return prefix + s_highDeathCountTaunts[rand() % 3];
	}

	// Level-specific taunts (50% chance to trigger, otherwise fall through to generic)
	if (rand() % 2 == 0)
	{
		switch (l_levelId)
		{
			case 3: return "The sand sends its regards.";
			case 5: return "Patience wasn't enough.";
			case 7: return "Solid ground is a myth.";
			case 9: return "Did you forget how to play? So did the controls.";
			case 10: return "Everything. All at once. All over.";
			default: break;
		}
	}

	// Fast death (wider window: <5 seconds)
	if (sm.levelTime < 5.0f)
		return s_fastDeathTaunts[rand() % s_fastDeathTauntCount];

	// Generic fallback
	return s_deathTaunts[rand() % s_deathTauntCount];
}

std::string GameOverState::SelectVictoryTaunt(int l_levelId, int l_stars)
{
	// Level 10 completion — special
	if (l_levelId == 10)
	{
		const char* l10[] = {
			"...You actually did it.",
			"The cruel world has nothing left to say.",
			"Everything. All at once. And you survived."
		};
		return l10[rand() % 3];
	}

	// Star-aware taunts
	if (l_stars == 3)
	{
		const char* star3[] = {
			"The cruel world tips its hat.",
			"Flawless. The world is furious.",
			"Three stars. The world didn't think you had it in you."
		};
		return star3[rand() % 3];
	}

	if (l_stars == 1)
	{
		const char* star1[] = {
			"You survived. Barely.",
			"That counts. Technically.",
			"A win is a win. A messy, ugly win."
		};
		return star1[rand() % 3];
	}

	// Generic victory (2 stars or default)
	return s_victoryTaunts[rand() % s_victoryTauntCount];
}

GameOverState::GameOverState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_statCount(5),
	  m_selectedItem(0),
	  m_itemCount(3),
	  m_hasNextLevel(false),
	  m_keyReleased(false)
{
}

void GameOverState::OnEnter()
{
	if (!m_font.loadFromFile(FONT_PATH))
		std::cerr << "GameOverState: Failed to load font" << std::endl;

	Window& window = m_stateManager.GetWindow();
	sf::Vector2u winSize = window.GetWindowSize();
	// Derive paper tone from the completed level
	int idx = m_stateManager.currentLevel - 1;
	const auto& levels = GetAllLevels();
	sf::Color paperBg = (idx >= 0 && idx < NUM_LEVELS)
		? levels[idx].paperTone : sf::Color(235, 225, 210);
	window.SetBackground(paperBg);

	bool won = m_stateManager.levelComplete;
	bool endless = m_stateManager.endlessMode;

	// Title
	m_title.setFont(m_font);
	if (endless)
		m_title.setString("Game Over");
	else
		m_title.setString(won ? "Level Complete!" : "You Died.");
	m_title.setCharacterSize(48);
	m_title.setFillColor(won ? sf::Color(45, 110, 55) : sf::Color(170, 55, 40));
	sf::FloatRect titleBounds = m_title.getLocalBounds();
	m_title.setPosition((winSize.x - titleBounds.width) / 2.0f, 80.f);

	// Star calculation (needed for victory taunt selection)
	int stars = 1;
	if (won && idx >= 0 && idx < NUM_LEVELS)
	{
		if (m_stateManager.selfCollisions <= levels[idx].starThreshold2)
			stars = 2;
		if (m_stateManager.selfCollisions <= levels[idx].starThreshold3)
			stars = 3;
	}

	// Context-sensitive taunt selection
	std::string tauntStr;
	int applesToWin = (idx >= 0 && idx < NUM_LEVELS) ? levels[idx].applesToWin : 0;
	if (won)
		tauntStr = SelectVictoryTaunt(m_stateManager.currentLevel, stars);
	else
		tauntStr = SelectDeathTaunt(m_stateManager.currentLevel, applesToWin);

	sf::Color goInk = (idx >= 0 && idx < NUM_LEVELS)
		? levels[idx].inkTint : sf::Color(60, 50, 45);
	sf::Color goLightInk(std::min(255, (int)goInk.r + 60),
						  std::min(255, (int)goInk.g + 50),
						  std::min(255, (int)goInk.b + 45));

	m_taunt.setFont(m_font);
	m_taunt.setString(tauntStr);
	m_taunt.setCharacterSize(20);
	m_taunt.setFillColor(goLightInk);
	sf::FloatRect tauntBounds = m_taunt.getLocalBounds();
	m_taunt.setPosition((winSize.x - tauntBounds.width) / 2.0f, 150.f);

	// Stats
	std::string statLabels[] = {
		"Score: " + std::to_string(m_stateManager.score),
		"Apples: " + std::to_string(m_stateManager.applesEaten),
		"Max Combo: x" + std::to_string(m_stateManager.combo),
		"",
		"Self Collisions: " + std::to_string(m_stateManager.selfCollisions)
	};

	// Time formatting
	int mins = (int)m_stateManager.levelTime / 60;
	int secs = (int)m_stateManager.levelTime % 60;
	std::ostringstream ts;
	ts << "Time: " << mins << ":" << std::setw(2) << std::setfill('0') << secs;
	statLabels[3] = ts.str();

	float statsStartY = 210.f;
	m_statCount = 5;
	for (int i = 0; i < 5; i++)
	{
		m_stats[i].setFont(m_font);
		m_stats[i].setString(statLabels[i]);
		m_stats[i].setCharacterSize(20);
		m_stats[i].setFillColor(goInk);
		sf::FloatRect bounds = m_stats[i].getLocalBounds();
		m_stats[i].setPosition((winSize.x - bounds.width) / 2.0f, statsStartY + i * 32.f);
	}

	// Star rating display for wins
	if (won)
	{
		std::string starStr = "";
		for (int i = 0; i < 3; i++)
			starStr += (i < stars) ? "* " : "- ";
		m_stats[4].setString(starStr);
		m_stats[4].setFillColor(sf::Color(200, 170, 30)); // Gold ink
		sf::FloatRect bounds = m_stats[4].getLocalBounds();
		m_stats[4].setPosition((winSize.x - bounds.width) / 2.0f, statsStartY + 4 * 32.f);
	}

	// Death screen enhancements (death counter + best attempt comparison)
	if (!won)
	{
		// Death counter with retry attempt
		std::string deathStr = "Death #" + std::to_string(m_stateManager.totalDeaths);
		if (m_stateManager.retryCount >= 3)
			deathStr += "  (Attempt " + std::to_string(m_stateManager.retryCount + 1) + ")";
		m_stats[5].setFont(m_font);
		m_stats[5].setString(deathStr);
		m_stats[5].setCharacterSize(17);
		m_stats[5].setFillColor(goLightInk);
		sf::FloatRect dBounds = m_stats[5].getLocalBounds();
		m_stats[5].setPosition((winSize.x - dBounds.width) / 2.0f, statsStartY + 5 * 32.f + 6.f);
		m_statCount = 6;

		// Best attempt comparison (show if we have a session best and it's better)
		if (m_stateManager.retryCount >= 2 &&
			m_stateManager.sessionBestApples > m_stateManager.deathAppleCount)
		{
			m_stats[6].setFont(m_font);
			m_stats[6].setString("Session best: " +
				std::to_string(m_stateManager.sessionBestApples) + " apples");
			m_stats[6].setCharacterSize(17);
			m_stats[6].setFillColor(goLightInk);
			sf::FloatRect bBounds = m_stats[6].getLocalBounds();
			m_stats[6].setPosition((winSize.x - bBounds.width) / 2.0f, statsStartY + 6 * 32.f + 6.f);
			m_statCount = 7;
		}
	}

	// Menu items — "Next Level" only on victory and if not the last level (and not endless)
	m_hasNextLevel = won && !endless && m_stateManager.currentLevel < NUM_LEVELS;

	std::vector<std::string> items;
	m_menuActions.clear();
	if (m_hasNextLevel)
	{
		items.push_back("Next Level");
		m_menuActions.push_back(GameOverAction::NextLevel);
	}
	items.push_back("Retry");
	m_menuActions.push_back(GameOverAction::Retry);
	if (!endless)
	{
		items.push_back("Level Select");
		m_menuActions.push_back(GameOverAction::LevelSelect);
	}
	items.push_back("Main Menu");
	m_menuActions.push_back(GameOverAction::MainMenu);
	m_itemCount = (int)items.size();

	// Save progress
	SaveManager::Save(m_stateManager, m_stateManager.GetStats(),
					  m_stateManager.GetAchievements());

	float menuStartY = statsStartY + m_statCount * 32.f + 40.f;
	float spacing = 46.f;

	for (int i = 0; i < m_itemCount; i++)
	{
		m_menuItems[i].setFont(m_font);
		m_menuItems[i].setString(items[i]);
		m_menuItems[i].setCharacterSize(28);
		sf::FloatRect bounds = m_menuItems[i].getLocalBounds();
		m_menuItems[i].setPosition((winSize.x - bounds.width) / 2.0f, menuStartY + i * spacing);
	}

	m_selectedItem = 0;
	m_keyReleased = false;

	// Screen shake on death
	if (!won)
		m_screenShake.Trigger(0.4f, 4.0f);
}

void GameOverState::OnExit()
{
	m_screenShake.Reset(m_stateManager.GetWindow());
}

void GameOverState::HandleInput()
{
	bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	bool rPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::R);

	if (!upPressed && !downPressed && !enterPressed && !rPressed)
	{
		m_keyReleased = true;
		return;
	}

	if (!m_keyReleased)
		return;

	m_keyReleased = false;

	// Quick restart
	if (rPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");
		m_stateManager.SwitchTo(StateType::Gameplay);
		return;
	}

	if (upPressed)
	{
		m_selectedItem--;
		if (m_selectedItem < 0)
			m_selectedItem = m_itemCount - 1;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (downPressed)
	{
		m_selectedItem++;
		if (m_selectedItem >= m_itemCount)
			m_selectedItem = 0;
		m_stateManager.GetAudio().PlaySound("menu_navigate");
	}
	else if (enterPressed)
	{
		m_stateManager.GetAudio().PlaySound("menu_select");

		if (m_selectedItem >= 0 && m_selectedItem < (int)m_menuActions.size())
		{
			switch (m_menuActions[m_selectedItem])
			{
				case GameOverAction::NextLevel:
					m_stateManager.currentLevel++;
					m_stateManager.SwitchTo(StateType::Gameplay);
					break;
				case GameOverAction::Retry:
					m_stateManager.SwitchTo(StateType::Gameplay);
					break;
				case GameOverAction::LevelSelect:
					m_stateManager.endlessMode = false;
					m_stateManager.SwitchTo(StateType::LevelSelect);
					break;
				case GameOverAction::MainMenu:
					m_stateManager.endlessMode = false;
					m_stateManager.SwitchTo(StateType::MainMenu);
					break;
				default:
					break;
			}
		}
	}
}

void GameOverState::Update(float l_dt)
{
	// Screen shake
	m_screenShake.Update(l_dt, m_stateManager.GetWindow());

	// Menu item colors
	for (int i = 0; i < m_itemCount; i++)
	{
		if (i == m_selectedItem)
			m_menuItems[i].setFillColor(sf::Color(170, 55, 40));
		else
			m_menuItems[i].setFillColor(sf::Color(100, 90, 80));
	}
}

void GameOverState::Render()
{
	Window& window = m_stateManager.GetWindow();

	window.Draw(m_title);
	window.Draw(m_taunt);

	for (int i = 0; i < m_statCount; i++)
		window.Draw(m_stats[i]);

	for (int i = 0; i < m_itemCount; i++)
		window.Draw(m_menuItems[i]);
}
