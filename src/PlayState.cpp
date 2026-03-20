#include "PlayState.h"
#include "AudioManager.h"
#include "InkRenderer.h"
#include <algorithm>
#include <string>
#include <cmath>

PlayState::PlayState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_world(l_stateManager.GetWindow(), m_snake),
	  m_hud(l_stateManager.GetWindow().GetWindowSize()),
	  m_predatorApplesEaten(0),
	  m_psychedelicTimer(0.0f),
	  m_elapsedTime(0.0f),
	  m_gameTime(0.0f),
	  m_applesEaten(0),
	  m_consecutiveApples(0),
	  m_speedModifier(1.0f),
	  m_lastShrinkCount(0),
	  m_cheatExtend(false),
	  m_escReleased(true),
	  m_rReleased(true),
	  m_comboSoundPlayed(false),
	  m_levelCompleteDelay(-1.0f),
	  m_cruelPhase(0),
	  m_screenFlipped(false),
	  m_phaseAnnouncementTimer(0.0f),
	  m_announcementFontLoaded(false),
	  m_postProcessorInited(false),
	  m_pageTurnTimer(0.0f),
	  m_pageTurnDuration(0.5f),
	  m_deathInkRunTimer(0.0f),
	  m_deathInkRunDuration(0.3f),
	  m_deathInkRunActive(false),
	  m_appleBurstTimer(0.0f),
	  m_borderHatchTimer(0.0f),
	  m_borderHatchDuration(0.3f)
{
}

void PlayState::OnEnter()
{
	Window& window = m_stateManager.GetWindow();

	// Load level config
	auto levels = GetAllLevels();
	int idx = m_stateManager.currentLevel - 1;
	if (idx < 0 || idx >= NUM_LEVELS) idx = 0;
	m_stateManager.currentLevel = idx + 1;
	m_levelConfig = levels[idx];

	// Apply level palette
	window.SetBackground(m_levelConfig.background);

	// Reset game state
	m_snake.Reset();
	m_world.SetTopOffset(HUD::GetHeight());
	m_world.Reset(window, m_snake);

	// Set shrink parameters before first RespawnApple so the pre-shrink
	// safety margin uses the configured interval, not the default
	m_world.SetShrinkInterval(m_levelConfig.shrinkInterval);
	m_world.SetShrinkTimerSec(m_levelConfig.shrinkTimerSec);

	m_world.RespawnApple(m_snake);
	m_world.SetBorderColor(m_levelConfig.border);
	m_hud.SetLevelColors(m_levelConfig.border, m_levelConfig.background);

	// Apply remaining level-specific configuration
	m_world.SetAppleColor(m_levelConfig.apple);
	m_snake.SetColors(m_levelConfig.snakeHead, m_levelConfig.snakeBody);

	m_elapsedTime = 0.0f;
	m_gameTime = 0.0f;
	m_applesEaten = 0;
	m_consecutiveApples = 0;
	m_speedModifier = 1.0f;
	m_lastShrinkCount = 0;
	m_cheatExtend = false;
	m_escReleased = true;
	m_rReleased = true;
	m_comboSoundPlayed = false;
	m_levelCompleteDelay = -1.0f;

	m_stateManager.score = 0;
	m_stateManager.applesEaten = 0;
	m_stateManager.combo = 0;
	m_stateManager.comboMultiplier = 1.0f;
	m_stateManager.selfCollisions = 0;
	m_stateManager.levelTime = 0.0f;
	m_stateManager.levelComplete = false;

	m_mirrorFlipCounter = 0;

	m_particles.Clear();
	m_screenShake.Reset(m_stateManager.GetWindow());

	// Initialize level mechanic systems
	if (m_levelConfig.hasBlackouts)
		m_blackout.Reset();

	if (m_levelConfig.hasQuicksand)
	{
		float maxThick = std::max({m_world.GetEffectiveThickness(0), m_world.GetEffectiveThickness(1),
								   m_world.GetEffectiveThickness(2), m_world.GetEffectiveThickness(3)});
		m_quicksand.Reset(m_world.GetMaxX(), m_world.GetMaxY(),
						  maxThick, m_snake.GetBlockSize(),
						  m_world.GetTopOffset());
	}

	if (m_levelConfig.hasMirrorGhost)
		m_mirrorGhost.Reset();

	if (m_levelConfig.hasTimedApples)
		m_timedApple.Reset(m_levelConfig.appleTimerSec);

	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.Reset(m_snake.GetBlockSize());
		m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());
	}

	if (m_levelConfig.hasEarthquakes)
		m_earthquake.Reset(m_snake.GetBlockSize());

	if (m_levelConfig.hasPredator)
	{
		m_predator.Reset(m_snake.GetBlockSize(), m_snake, m_world);
		m_predatorApplesEaten = 0;
	}

	if (m_levelConfig.hasControlShuffle)
		m_controlShuffle.Reset();

	m_psychedelicTimer = 0.0f;

	// Level 10 "Cruel World": override to Phase 1 after all mechanics initialized
	if (m_levelConfig.id == 10)
		InitCruelWorldPhases();

	// --- Initialize "Living Ink" visual systems ---
	m_snake.SetUseInkStyle(true);
	m_snake.SetCorruption(m_levelConfig.corruption);
	m_snake.SetInkTint(m_levelConfig.inkTint);

	m_world.SetUseInkStyle(true);
	m_world.SetCorruption(m_levelConfig.corruption);
	m_world.SetInkTint(m_levelConfig.inkTint);
	m_world.SetAccentColor(m_levelConfig.accentColor);

	// Generate paper background
	sf::Vector2u winSize = window.GetWindowSize();
	m_paperBackground.Generate(m_levelConfig, winSize.x, winSize.y);

	// Initialize post-processor (only once)
	if (!m_postProcessorInited)
	{
		m_postProcessorInited = m_postProcessor.Init(winSize.x, winSize.y);
	}
	m_postProcessor.Configure(m_levelConfig);

	// Start page-turn entry animation
	m_pageTurnTimer = m_pageTurnDuration;
	m_deathInkRunActive = false;
	m_deathInkRunTimer = 0.0f;
	m_appleBurstTimer = 0.0f;
	m_borderHatchTimer = 0.0f;
}

void PlayState::InitCruelWorldPhases()
{
	m_cruelPhase = 0;
	m_screenFlipped = false;
	m_phaseAnnouncementTimer = 0.0f;
	m_phaseAnnouncementText.clear();

	// Load announcement font once
	if (!m_announcementFontLoaded)
	{
		if (m_announcementFont.loadFromFile(FONT_PATH))
			m_announcementFontLoaded = true;
	}

	// Phase 1: only timed apples active
	m_levelConfig.hasBlackouts = false;
	m_levelConfig.hasQuicksand = false;
	m_levelConfig.hasPredator = false;
	m_levelConfig.hasEarthquakes = false;
	m_levelConfig.hasControlShuffle = false;
	m_levelConfig.hasPoisonApples = false;
	m_levelConfig.hasTimedApples = true;
	m_levelConfig.hasMirrorGhost = false;

	// Phase 1 parameters
	m_levelConfig.baseSpeed = 12.0f;
	m_levelConfig.shrinkInterval = 4;
	m_levelConfig.shrinkTimerSec = 0.0f;
	m_levelConfig.appleTimerSec = 6.0f;
	m_world.SetShrinkInterval(4);
	m_world.SetShrinkTimerSec(0.0f);

	// Phase 1 theme: warm maroon (same as Level 1 "False Hope")
	m_levelConfig.background = sf::Color(30, 15, 20);
	m_levelConfig.border = sf::Color(200, 100, 50);
	m_stateManager.GetWindow().SetBackground(m_levelConfig.background);
	m_world.SetBorderColor(m_levelConfig.border);
	m_hud.SetLevelColors(m_levelConfig.border, m_levelConfig.background);
}

void PlayState::AdvanceCruelPhase()
{
	m_cruelPhase++;
	Window& window = m_stateManager.GetWindow();

	switch (m_cruelPhase)
	{
		case 1: // Phase 2 (apples 6-10): Blackouts + Predator
		{
			m_levelConfig.hasBlackouts = true;
			m_levelConfig.hasPredator = true;

			m_blackout.Reset();
			m_predator.Reset(m_snake.GetBlockSize(), m_snake, m_world);
			m_predatorApplesEaten = 0;

			m_levelConfig.appleTimerSec = 5.0f;
			m_levelConfig.baseSpeed = 13.0f;
			m_levelConfig.shrinkInterval = 3;
			m_world.SetShrinkInterval(3);

			// Theme: cold blue-gray (Level 8 palette)
			m_levelConfig.background = sf::Color(15, 15, 25);
			m_levelConfig.border = sf::Color(60, 70, 100);
			window.SetBackground(m_levelConfig.background);
			m_world.SetBorderColor(m_levelConfig.border);
			m_hud.SetLevelColors(m_levelConfig.border, m_levelConfig.background);

			m_phaseAnnouncementText = "It gets worse.";
			m_phaseAnnouncementTimer = 2.0f;
			break;
		}

		case 2: // Phase 3 (apples 11-15): Quicksand + Poison
		{
			m_levelConfig.hasQuicksand = true;
			m_levelConfig.hasPoisonApples = true;

			float maxThick = std::max({m_world.GetEffectiveThickness(0),
									   m_world.GetEffectiveThickness(1),
									   m_world.GetEffectiveThickness(2),
									   m_world.GetEffectiveThickness(3)});
			m_quicksand.Reset(m_world.GetMaxX(), m_world.GetMaxY(),
							  maxThick, m_snake.GetBlockSize(),
							  m_world.GetTopOffset());

			m_poisonApple.Reset(m_snake.GetBlockSize());
			m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());

			m_levelConfig.appleTimerSec = 4.0f;
			m_levelConfig.baseSpeed = 14.0f;
			m_levelConfig.shrinkInterval = 2;
			m_world.SetShrinkInterval(2);

			// Theme: sickly poisonous green
			m_levelConfig.background = sf::Color(10, 30, 10);
			m_levelConfig.border = sf::Color(40, 120, 30);
			window.SetBackground(m_levelConfig.background);
			m_world.SetBorderColor(m_levelConfig.border);
			m_hud.SetLevelColors(m_levelConfig.border, m_levelConfig.background);

			m_phaseAnnouncementText = "It gets worse.";
			m_phaseAnnouncementTimer = 2.0f;
			break;
		}

		case 3: // Phase 4 (apples 16-20): Everything. All at once.
		{
			m_levelConfig.hasEarthquakes = true;
			m_levelConfig.hasControlShuffle = true;
			m_levelConfig.hasMirrorGhost = true;

			m_earthquake.Reset(m_snake.GetBlockSize());
			m_controlShuffle.Reset();
			m_mirrorGhost.Reset();
			m_mirrorFlipCounter = 0;

			m_levelConfig.appleTimerSec = 3.0f;
			m_levelConfig.baseSpeed = 15.0f;
			m_levelConfig.shrinkInterval = 2;
			m_levelConfig.shrinkTimerSec = 5.0f;
			m_world.SetShrinkInterval(2);
			m_world.SetShrinkTimerSec(5.0f);

			// Theme: near-black with crimson borders
			m_levelConfig.background = sf::Color(8, 5, 5);
			m_levelConfig.border = sf::Color(180, 20, 20);
			window.SetBackground(m_levelConfig.background);
			m_world.SetBorderColor(m_levelConfig.border);
			m_hud.SetLevelColors(m_levelConfig.border, m_levelConfig.background);

			m_phaseAnnouncementText = "Everything. All at once.";
			m_phaseAnnouncementTimer = 2.0f;
			break;
		}

		default:
			break;
	}

	// Common phase transition effects
	m_screenShake.Trigger(0.5f, 5.0f);
	m_stateManager.GetAudio().PlaySound("phase_advance");

	// Update ink-style visuals for the new phase
	// Map L10 phases to escalating corruption and different paper tones
	static const sf::Color phasePaper[] = {
		sf::Color(245, 235, 220), // Phase 1: warm cream
		sf::Color(210, 215, 225), // Phase 2: cold gray
		sf::Color(200, 210, 190), // Phase 3: sickly green
		sf::Color(200, 180, 170), // Phase 4: scorched
	};
	static const sf::Color phaseInk[] = {
		sf::Color(60, 50, 45),    // Phase 1: graphite
		sf::Color(40, 45, 70),    // Phase 2: slate blue
		sf::Color(20, 50, 20),    // Phase 3: forest
		sf::Color(80, 20, 15),    // Phase 4: blood
	};
	static const float phaseCorruption[] = { 0.15f, 0.40f, 0.65f, 1.0f };

	int pi = std::min(m_cruelPhase, 3);
	m_levelConfig.paperTone = phasePaper[pi];
	m_levelConfig.inkTint = phaseInk[pi];
	m_levelConfig.corruption = phaseCorruption[pi];

	// Regenerate paper background for new phase
	sf::Vector2u winSize = m_stateManager.GetWindow().GetWindowSize();
	m_paperBackground.Generate(m_levelConfig, winSize.x, winSize.y);

	// Update ink params on snake and world
	m_snake.SetCorruption(m_levelConfig.corruption);
	m_snake.SetInkTint(m_levelConfig.inkTint);
	m_world.SetCorruption(m_levelConfig.corruption);
	m_world.SetInkTint(m_levelConfig.inkTint);

	// Reconfigure post-processor for new corruption level
	m_postProcessor.Configure(m_levelConfig);
}

void PlayState::OnExit()
{
	m_screenShake.Reset(m_stateManager.GetWindow());
}

void PlayState::HandleInput()
{
	// Debounced Pause (Escape)
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
	{
		if (m_escReleased)
		{
			m_escReleased = false;
			m_stateManager.PushState(StateType::Pause);
			return;
		}
	}
	else
	{
		m_escReleased = true;
	}

	// Debounced Quick restart (R)
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		if (m_rReleased)
		{
			m_rReleased = false;
			OnEnter(); // restart level
			return;
		}
	}
	else
	{
		m_rReleased = true;
	}

	// Determine desired direction from input
	Direction inputDir = Direction::None;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		inputDir = Direction::Up;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		inputDir = Direction::Down;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		inputDir = Direction::Right;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		inputDir = Direction::Left;

	// Poison: invert controls when active
	if (inputDir != Direction::None && m_levelConfig.hasPoisonApples && m_poisonApple.IsControlInverted())
	{
		switch (inputDir)
		{
			case Direction::Up:    inputDir = Direction::Down;  break;
			case Direction::Down:  inputDir = Direction::Up;    break;
			case Direction::Left:  inputDir = Direction::Right; break;
			case Direction::Right: inputDir = Direction::Left;  break;
			default: break;
		}
	}

	// Control shuffle: remap direction
	if (inputDir != Direction::None && m_levelConfig.hasControlShuffle)
		inputDir = m_controlShuffle.MapDirection(inputDir);

	// Apply direction (prevent 180-degree reversal)
	if (inputDir != Direction::None)
	{
		Direction cur = m_snake.GetDirection();
		bool valid = true;
		if (inputDir == Direction::Up && cur == Direction::Down) valid = false;
		if (inputDir == Direction::Down && cur == Direction::Up) valid = false;
		if (inputDir == Direction::Left && cur == Direction::Right) valid = false;
		if (inputDir == Direction::Right && cur == Direction::Left) valid = false;
		if (valid)
			m_snake.SetDirection(inputDir);
	}

	// Cheat code
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		m_cheatExtend = true;
}

void PlayState::Update(float l_dt)
{
	m_elapsedTime += l_dt;
	m_gameTime += l_dt;
	m_snake.UpdateVisuals(l_dt);
	m_postProcessor.Update(l_dt);

	// Transition animation timers
	if (m_pageTurnTimer > 0.0f)
		m_pageTurnTimer -= l_dt;
	if (m_deathInkRunActive)
	{
		m_deathInkRunTimer += l_dt;
		if (m_deathInkRunTimer >= m_deathInkRunDuration)
			m_deathInkRunActive = false;
	}
	if (m_appleBurstTimer > 0.0f)
		m_appleBurstTimer -= l_dt;
	if (m_borderHatchTimer > 0.0f)
		m_borderHatchTimer -= l_dt;

	float speed = m_levelConfig.baseSpeed;
	// Speed creep: +0.5 every 5 apples
	speed += (m_applesEaten / 5) * 0.5f;
	speed *= m_speedModifier;

	float timeStep = 1.0f / speed;

	if (m_elapsedTime >= timeStep && m_levelCompleteDelay < 0.0f)
	{
		Window& window = m_stateManager.GetWindow();

		// Capture head position before tick for accurate VFX placement
		Position preTickHead = m_snake.GetPosition();

		m_world.Update(window, m_snake);
		m_snake.Tick(window.GetWindowSize());

		// Wall collision grace: forgive wall death right after a control shuffle
		if (m_snake.HasLost())
		{
			if (m_levelConfig.hasControlShuffle && m_controlShuffle.IsGracePeriod())
				m_snake.LoseStatus(false);
			else
			{
				OnDeath();
				return;
			}
		}

		// Detect apple eaten by comparing count
		int newApplesEaten = m_world.GetApplesEaten();
		if (newApplesEaten > m_applesEaten)
		{
			m_applesEaten = newApplesEaten;
			OnAppleEaten(preTickHead);
		}

		// Detect world shrink and award bonus
		int newShrinkCount = m_world.GetShrinkCount();
		if (newShrinkCount > m_lastShrinkCount)
		{
			m_stateManager.score += 250; // survive world shrink bonus
			m_lastShrinkCount = newShrinkCount;
			m_stateManager.GetAudio().PlaySound("world_shrink");
			m_screenShake.Trigger(0.3f, 3.0f);
			m_world.FlashBorders(0.2f);
			m_borderHatchTimer = m_borderHatchDuration; // Trigger hatch fill animation
		}

		// Check for self-collision
		if (m_snake.DidSelfCollide())
		{
			m_stateManager.selfCollisions++;
			m_stateManager.score = std::max(0, m_stateManager.score - 50);
			UpdateCombo(true);
			m_stateManager.GetAudio().PlaySound("self_collide");
			m_particles.SpawnSelfCollisionCut(m_snake.GetLastCutSegments(), m_snake.GetBlockSize(),
										 m_levelConfig.snakeBody);
			m_snake.ClearSelfCollideFlag();
		}

		// Mirror ghost update (tick-based, same rate as snake movement)
		if (m_levelConfig.hasMirrorGhost)
		{
			float bs = m_snake.GetBlockSize();
			float centerX = (m_world.GetEffectiveThickness(3) / bs +
							 m_world.GetMaxX() - m_world.GetEffectiveThickness(1) / bs - 1) / 2.0f;
			float centerY = ((m_world.GetEffectiveThickness(0) + m_world.GetTopOffset()) / bs +
							 m_world.GetMaxY() - m_world.GetEffectiveThickness(2) / bs - 1) / 2.0f;
			m_mirrorGhost.Update(m_snake, centerX, centerY);

			if (m_mirrorGhost.CheckCollision(m_snake.GetPosition()))
				m_snake.LoseStatus(true);
		}

		// Check poison apple collision
		if (m_levelConfig.hasPoisonApples)
		{
			if (m_poisonApple.CheckCollision(m_snake.GetPosition()))
			{
				m_poisonApple.OnPoisonEaten();
				m_stateManager.score = std::max(0, m_stateManager.score - 200);
				UpdateCombo(true);

				sf::Vector2f poisonPixelPos = m_poisonApple.GetPixelPos(m_snake.GetBlockSize());
				m_particles.SpawnAppleBurst(poisonPixelPos, sf::Color::Magenta);
				m_particles.SpawnFloatingText("-200", poisonPixelPos, sf::Color(255, 50, 100));
				m_stateManager.GetAudio().PlaySound("self_collide");

				for (int i = 0; i < m_poisonApple.GetGrowAmount(); i++)
					m_snake.Extend();

				m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());
			}
		}

		// Check predator collision with player
		if (m_levelConfig.hasPredator)
		{
			if (m_predator.HitPlayer(m_snake.GetPosition()))
				m_snake.LoseStatus(true);
		}

		// Check death (entity collisions — not forgiven by grace)
		if (m_snake.HasLost())
		{
			OnDeath();
			return;
		}

		// Cheat code
		if (m_cheatExtend)
		{
			m_snake.Extend();
			m_cheatExtend = false;
		}

		m_elapsedTime -= timeStep;
	}

	// Update HUD
	m_stateManager.levelTime = m_gameTime;
	m_hud.Update(m_stateManager.score, m_stateManager.comboMultiplier,
				 m_applesEaten, m_levelConfig.applesToWin,
				 m_levelConfig.name, m_gameTime, l_dt,
				 m_levelConfig.hasPredator ? m_predatorApplesEaten : -1);

	// Update visual effects (continuous, not tick-based)
	m_particles.Update(l_dt);
	m_screenShake.Update(l_dt, m_stateManager.GetWindow());
	m_world.UpdateFlash(l_dt);

	// --- Level mechanic continuous updates ---

	// Reset speed modifier each frame; mechanics below accumulate into it
	m_speedModifier = 1.0f;

	// Blackout (Level 2)
	if (m_levelConfig.hasBlackouts)
	{
		m_blackout.Update(l_dt, m_snake);
		if (m_blackout.JustStartedBlackout())
		{
			m_world.RespawnApple(m_snake);
			m_stateManager.GetAudio().PlaySound("blackout_on");
		}
	}

	// Quicksand speed modifier (Level 3)
	if (m_levelConfig.hasQuicksand)
	{
		float maxThick = std::max({m_world.GetEffectiveThickness(0), m_world.GetEffectiveThickness(1),
								   m_world.GetEffectiveThickness(2), m_world.GetEffectiveThickness(3)});
		m_quicksand.Update(l_dt, m_world.GetMaxX(), m_world.GetMaxY(),
						   maxThick, m_snake.GetBlockSize(),
						   m_world.GetTopOffset());

		if (m_quicksand.IsOnQuicksand(m_snake.GetPosition()))
			m_speedModifier *= 0.5f;
	}

	// Timer-based world shrinking (Level 3)
	if (m_levelConfig.shrinkTimerSec > 0.0f && m_levelCompleteDelay < 0.0f)
	{
		Window& window = m_stateManager.GetWindow();
		m_world.UpdateTimedShrink(l_dt, window, m_snake);
		// Shrink may have moved borders onto the snake
		m_world.CheckCollision(window, m_snake);
		if (m_snake.HasLost()) { OnDeath(); return; }
	}

	// Poison apples (Level 6)
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.Update(l_dt);
		if (m_poisonApple.GetSpeedMultiplier() > 1.0f)
			m_speedModifier *= m_poisonApple.GetSpeedMultiplier();

		// Snake color flash when poisoned (driven from Update, not Render)
		if (m_poisonApple.IsControlInverted())
		{
			float flash = std::sin(m_gameTime * 10.0f);
			if (flash > 0)
				m_snake.SetColors(sf::Color::Magenta, sf::Color(200, 0, 100));
			else
				m_snake.SetColors(m_levelConfig.snakeHead, m_levelConfig.snakeBody);
		}
		else
		{
			m_snake.SetColors(m_levelConfig.snakeHead, m_levelConfig.snakeBody);
		}
	}

	// Timed apples (Level 5)
	if (m_levelConfig.hasTimedApples && m_levelCompleteDelay < 0.0f)
	{
		m_timedApple.Update(l_dt);

		if (m_timedApple.HasExpired())
		{
			// Don't penalize if the snake is already on the apple (will be
			// eaten on the next tick — timer expired between ticks)
			sf::Vector2f ap = m_world.GetApplePos();
			Position head = m_snake.GetPosition();
			if (head.x == (int)ap.x && head.y == (int)ap.y)
			{
				m_timedApple.OnAppleEaten(GetAppleTimerDuration());
			}
			else
			{
				// Penalty: apple missed, world shrinks
				Window& window = m_stateManager.GetWindow();
				m_world.TriggerShrink(window, m_snake);
				m_lastShrinkCount = m_world.GetShrinkCount();
				m_stateManager.GetAudio().PlaySound("apple_miss");
				m_screenShake.Trigger(0.3f, 3.0f);
				m_world.FlashBorders(0.2f);

				// Check if shrink crushed the snake
				m_world.CheckCollision(window, m_snake);
				if (m_snake.HasLost()) { OnDeath(); return; }

				// Respawn apple with adjusted timer
				m_world.RespawnApple(m_snake);
				m_timedApple.OnAppleEaten(GetAppleTimerDuration());
			}
		}
	}

	// Earthquake (Level 7)
	if (m_levelConfig.hasEarthquakes && m_levelCompleteDelay < 0.0f)
	{
		Window& window = m_stateManager.GetWindow();
		m_earthquake.Update(l_dt, m_world, window);

		if (m_earthquake.IsWarning())
			m_screenShake.Trigger(0.1f, 1.5f);

		if (m_earthquake.JustQuaked())
		{
			m_screenShake.Trigger(0.6f, 6.0f);
			m_stateManager.GetAudio().PlaySound("earthquake");
			m_world.FlashBorders(0.3f);

			// Only respawn apple/poison if they're now inside a wall
			if (!m_world.IsAppleInBounds(m_snake.GetBlockSize()))
				m_world.RespawnApple(m_snake);

			if (m_levelConfig.hasPoisonApples && !m_poisonApple.IsInBounds(m_world, m_snake.GetBlockSize()))
				m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());

			m_world.CheckCollision(window, m_snake);
			if (m_snake.HasLost()) { OnDeath(); return; }
		}
	}

	// Predator (Level 8)
	if (m_levelConfig.hasPredator && m_levelCompleteDelay < 0.0f)
	{
		m_predator.Update(l_dt, m_world, m_snake);

		// Check if predator moved onto player head
		if (m_predator.HitPlayer(m_snake.GetPosition()))
		{
			m_snake.LoseStatus(true);
			OnDeath();
			return;
		}

		if (m_predator.JustAteApple())
		{
			m_predatorApplesEaten++;

			// World shrinks when predator eats apple
			Window& window = m_stateManager.GetWindow();
			m_world.TriggerShrink(window, m_snake);
			m_lastShrinkCount = m_world.GetShrinkCount();
			m_stateManager.GetAudio().PlaySound("predator_eat");
			m_screenShake.Trigger(0.3f, 3.0f);
			m_world.FlashBorders(0.2f);

			// Score penalty
			m_stateManager.score = std::max(0, m_stateManager.score - 150);
			sf::Vector2f ap(m_world.GetApplePos().x * m_snake.GetBlockSize(),
							m_world.GetApplePos().y * m_snake.GetBlockSize());
			m_particles.SpawnFloatingText("-150", ap, sf::Color(100, 100, 255));

			// Respawn apple
			m_world.RespawnApple(m_snake);

			// Lose condition: predator ate 5 apples
			if (m_predatorApplesEaten >= 5)
				m_snake.LoseStatus(true);

			// Check if shrink crushed the snake
			m_world.CheckCollision(window, m_snake);
			if (m_snake.HasLost()) { OnDeath(); return; }
		}

		if (m_predator.JustStartedHunting())
		{
			m_stateManager.GetAudio().PlaySound("predator_hunt");
			m_screenShake.Trigger(0.5f, 4.0f);
		}
	}

	// Control shuffle (Level 9)
	if (m_levelConfig.hasControlShuffle && m_levelCompleteDelay < 0.0f)
	{
		m_controlShuffle.Update(l_dt);

		if (m_controlShuffle.IsWarning())
			m_stateManager.GetAudio().PlaySound("shuffle_warning");

		if (m_controlShuffle.JustShuffled())
		{
			m_stateManager.GetAudio().PlaySound("control_shuffle");
			m_screenShake.Trigger(0.3f, 3.0f);
		}
	}

	// Border pulse during grace period
	if (m_levelConfig.hasControlShuffle && m_controlShuffle.IsGracePeriod())
	{
		float pulse = std::sin(m_gameTime * 20.0f);
		sf::Uint8 g = (sf::Uint8)(200 + 55 * pulse);
		m_world.SetBorderColor(sf::Color(100, g, 255));
	}
	else if (m_levelConfig.hasControlShuffle)
	{
		m_world.SetBorderColor(m_levelConfig.border);
	}

	// Psychedelic color cycling (Level 9 theme)
	if (m_levelConfig.id == 9 && m_levelCompleteDelay < 0.0f)
	{
		m_psychedelicTimer += l_dt;

		// Background tint cycles between purple and teal (applied as overlay in Render)
		// Apple: RGB cycling via phase-shifted sin waves
		float ap = m_psychedelicTimer * 2.0f;
		m_world.SetAppleColor(sf::Color(
			(sf::Uint8)(128 + 127 * std::sin(ap)),
			(sf::Uint8)(128 + 127 * std::sin(ap + 2.094f)),
			(sf::Uint8)(128 + 127 * std::sin(ap + 4.189f))));
	}

	// Level 10 phase announcement timer
	if (m_levelConfig.id == 10 && m_phaseAnnouncementTimer > 0.0f)
		m_phaseAnnouncementTimer -= l_dt;

	// Deferred level-complete transition (lets particles render first)
	if (m_levelCompleteDelay >= 0.0f)
	{
		m_levelCompleteDelay -= l_dt;
		if (m_levelCompleteDelay < 0.0f)
			m_stateManager.SwitchTo(StateType::GameOver);
	}
}

void PlayState::Render()
{
	Window& window = m_stateManager.GetWindow();
	bool usePostProcess = m_postProcessor.IsAvailable();

	// Begin post-processing capture (game scene renders to offscreen RT)
	if (usePostProcess)
		m_postProcessor.Begin();

	sf::RenderTarget& target = usePostProcess
		? m_postProcessor.GetTarget()
		: (sf::RenderTarget&)window.GetRenderWindow();

	// Draw paper background with page-turn entry animation
	if (m_paperBackground.IsGenerated())
	{
		if (m_pageTurnTimer > 0.0f)
		{
			// Page slides in from the right
			float progress = m_pageTurnTimer / m_pageTurnDuration; // 1.0 → 0.0
			float slideOffset = progress * (float)window.GetWindowSize().x;

			// Save view, offset for slide
			sf::View slideView = target.getView();
			sf::View offsetView = slideView;
			offsetView.move(slideOffset, 0);
			target.setView(offsetView);
			m_paperBackground.Render(target);
			target.setView(slideView);
		}
		else
		{
			m_paperBackground.Render(target);
		}
	}

	// Level 9: Psychedelic tint overlay cycling on paper background
	if (m_levelConfig.id == 9 && m_psychedelicTimer > 0.0f)
	{
		float t = (std::sin(m_psychedelicTimer * 0.8f) + 1.0f) / 2.0f;
		sf::Uint8 r = (sf::Uint8)(80 + t * 60);
		sf::Uint8 g = (sf::Uint8)(30 + (1.0f - t) * 60);
		sf::Uint8 b = (sf::Uint8)(100 + t * 30);
		sf::RectangleShape psychOverlay(sf::Vector2f(
			(float)window.GetWindowSize().x, (float)window.GetWindowSize().y));
		psychOverlay.setFillColor(sf::Color(r, g, b, 40)); // Subtle tint wash
		target.draw(psychOverlay);
	}

	// Level 10: screen flip (The Cruel Twist at apple 19)
	sf::View savedView;
	if (m_screenFlipped)
	{
		savedView = window.GetRenderWindow().getView();
		sf::View flipped = savedView;
		flipped.setRotation(180.f);
		if (usePostProcess)
			m_postProcessor.GetTarget().setView(flipped);
		else
			window.SetView(flipped);
	}

	m_world.RenderInk(target, m_gameTime);

	if (m_levelConfig.hasEarthquakes)
		m_earthquake.RenderTo(target, m_world);

	if (m_levelConfig.hasQuicksand)
		m_quicksand.RenderTo(target, m_snake.GetBlockSize());

	if (m_levelConfig.hasTimedApples)
	{
		sf::Vector2f applePixelPos(
			m_world.GetApplePos().x * m_snake.GetBlockSize(),
			m_world.GetApplePos().y * m_snake.GetBlockSize());
		m_timedApple.RenderTo(target, applePixelPos, m_snake.GetBlockSize() / 2.0f);
	}

	if (m_levelConfig.hasMirrorGhost)
	{
		float bs = m_snake.GetBlockSize();
		int bMinX = (int)(m_world.GetEffectiveThickness(3) / bs);
		int bMaxX = (int)(m_world.GetMaxX() - m_world.GetEffectiveThickness(1) / bs - 1);
		int bMinY = (int)((m_world.GetEffectiveThickness(0) + m_world.GetTopOffset()) / bs);
		int bMaxY = (int)(m_world.GetMaxY() - m_world.GetEffectiveThickness(2) / bs - 1);
		m_mirrorGhost.RenderTo(target, bs, bMinX, bMaxX, bMinY, bMaxY);
	}

	// Poison apple rendering + Phase 2 real apple pulse
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.RenderTo(target, m_snake.GetBlockSize());

		// In Phase 2, make the real apple pulse too
		if (m_poisonApple.GetRealApplesEaten() >= 8)
		{
			float pulseRadius = (m_snake.GetBlockSize() / 2.0f) + std::sin(m_gameTime * 4.0f) * 1.0f;
			float baseRadius = m_snake.GetBlockSize() / 2.0f;
			m_realPulse.setRadius(pulseRadius);
			m_realPulse.setOrigin(pulseRadius - baseRadius, pulseRadius - baseRadius);
			m_realPulse.setFillColor(m_levelConfig.apple);
			m_realPulse.setPosition(m_world.GetApplePos().x * m_snake.GetBlockSize(),
									m_world.GetApplePos().y * m_snake.GetBlockSize());
			target.draw(m_realPulse);
		}
	}

	if (m_levelConfig.hasPredator)
		m_predator.RenderTo(target, m_snake.GetBlockSize());

	// Snake: render with ink style to the post-process target
	m_snake.RenderInk(target);

	// Apple burst outline effect (expanding circle on eat)
	if (m_appleBurstTimer > 0.0f)
	{
		float progress = 1.0f - (m_appleBurstTimer / 0.2f); // 0→1
		float burstRadius = m_snake.GetBlockSize() * (1.0f + progress * 1.5f);
		sf::Uint8 burstAlpha = (sf::Uint8)(180 * (1.0f - progress));
		sf::Color burstOutline(m_appleBurstColor.r, m_appleBurstColor.g,
							   m_appleBurstColor.b, burstAlpha);
		float cx = m_appleBurstPos.x + m_snake.GetBlockSize() * 0.5f;
		float cy = m_appleBurstPos.y + m_snake.GetBlockSize() * 0.5f;
		InkRenderer::DrawWobblyCircle(target, cx, cy, burstRadius,
									  sf::Color::Transparent, burstOutline,
									  1.5f, m_levelConfig.corruption * 0.5f,
									  (unsigned int)(m_gameTime * 100.0f), 12);
	}

	// Border hatch fill animation (rapid strokes appearing in new border area)
	if (m_borderHatchTimer > 0.0f)
	{
		float progress = 1.0f - (m_borderHatchTimer / m_borderHatchDuration); // 0→1
		int strokeCount = (int)(progress * 15);
		sf::Color hatchColor(m_levelConfig.inkTint.r, m_levelConfig.inkTint.g,
							 m_levelConfig.inkTint.b, (sf::Uint8)(100 * (1.0f - progress)));
		unsigned int seed = (unsigned int)(m_gameTime * 50.0f);
		for (int s = 0; s < strokeCount; s++)
		{
			unsigned int h = InkRenderer::Hash(seed, (unsigned int)s);
			float sx = (float)(h % window.GetWindowSize().x);
			float sy = (float)((h >> 8) % window.GetWindowSize().y);
			float len = 8.0f + (float)((h >> 16) % 12);
			bool horiz = (h >> 28) & 1;
			if (horiz)
				InkRenderer::DrawWobblyLine(target, sx, sy, sx + len, sy,
											hatchColor, 1.0f, 0.3f, h);
			else
				InkRenderer::DrawWobblyLine(target, sx, sy, sx, sy + len,
											hatchColor, 1.0f, 0.3f, h);
		}
	}

	m_particles.RenderTo(target);

	if (m_levelConfig.hasBlackouts)
		m_blackout.RenderTo(target, window.GetWindowSize(), m_snake.GetBlockSize());

	// Restore un-flipped view for HUD and UI overlays (never upside-down)
	if (m_screenFlipped)
	{
		if (usePostProcess)
			m_postProcessor.GetTarget().setView(savedView);
		else
			window.SetView(savedView);
	}

	// End post-processing capture and apply shader chain
	if (usePostProcess)
	{
		m_postProcessor.End();
		m_postProcessor.Apply(window);
	}

	// HUD and overlays render directly to window (no post-processing, stays crisp)
	if (m_levelConfig.hasControlShuffle)
		m_controlShuffle.Render(window);

	m_hud.Render(window);

	// Level 10 phase announcement overlay (always right-side-up, on top)
	if (m_levelConfig.id == 10 && m_phaseAnnouncementTimer > 0.0f)
	{
		window.SetView(window.GetDefaultView());
		RenderPhaseAnnouncement(window);
	}
}

void PlayState::OnAppleEaten(const Position& l_applePos)
{
	m_consecutiveApples++;
	UpdateCombo(false);

	int points = CalculatePoints(100);
	m_stateManager.score += points;
	m_stateManager.applesEaten = m_applesEaten;

	// Audio + visual feedback
	m_stateManager.GetAudio().PlaySound("apple_eat");
	sf::Vector2f applePixelPos(
		l_applePos.x * m_snake.GetBlockSize(),
		l_applePos.y * m_snake.GetBlockSize());
	m_particles.SpawnAppleBurst(applePixelPos, m_levelConfig.apple);
	m_particles.SpawnFloatingText("+" + std::to_string(points), applePixelPos,
								  sf::Color(180, 140, 30));

	// Apple burst outline effect
	m_appleBurstTimer = 0.2f;
	m_appleBurstPos = applePixelPos;
	m_appleBurstColor = m_levelConfig.apple;

	// Mirror ghost: flip axis every 5 apples
	if (m_levelConfig.hasMirrorGhost)
	{
		m_mirrorFlipCounter++;
		if (m_mirrorFlipCounter % 5 == 0)
		{
			m_mirrorGhost.FlipAxis();
			m_stateManager.GetAudio().PlaySound("mirror_flip");
			m_screenShake.Trigger(0.2f, 2.0f);
		}
	}

	// Poison apples: notify real apple eaten, respawn poison
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.OnRealAppleEaten();
		m_poisonApple.SpawnPoison(m_snake, m_world, m_snake.GetBlockSize());
	}

	// Predator: notify that player got the apple
	if (m_levelConfig.hasPredator)
		m_predator.OnPlayerAteApple();

	// Control shuffle: update apple counter for indicator cutoff
	if (m_levelConfig.hasControlShuffle)
		m_controlShuffle.OnAppleEaten(m_applesEaten);

	// Timed apples: reset timer with adjusted duration
	if (m_levelConfig.hasTimedApples)
		m_timedApple.OnAppleEaten(GetAppleTimerDuration());

	// Level 10 "Cruel World" phase transitions
	if (m_levelConfig.id == 10)
	{
		bool phaseChanged = false;
		if (m_applesEaten == 5 && m_cruelPhase == 0)
			{ AdvanceCruelPhase(); phaseChanged = true; }
		else if (m_applesEaten == 10 && m_cruelPhase == 1)
			{ AdvanceCruelPhase(); phaseChanged = true; }
		else if (m_applesEaten == 15 && m_cruelPhase == 2)
			{ AdvanceCruelPhase(); phaseChanged = true; }

		// Re-set timed apple with new phase duration
		if (phaseChanged && m_levelConfig.hasTimedApples)
			m_timedApple.OnAppleEaten(GetAppleTimerDuration());

		// The Cruel Twist: screen flips at apple 19
		if (m_applesEaten == 19 && !m_screenFlipped)
		{
			m_screenFlipped = true;
			m_screenShake.Trigger(0.8f, 8.0f);
			m_stateManager.GetAudio().PlaySound("phase_advance");
		}
	}

	// Level 1 "False Hope" twist: double shrink on final apple
	if (m_levelConfig.id == 1 && m_applesEaten == m_levelConfig.applesToWin)
	{
		Window& window = m_stateManager.GetWindow();
		m_world.TriggerShrink(window, m_snake);
		m_world.TriggerShrink(window, m_snake);
		m_lastShrinkCount = m_world.GetShrinkCount();
		m_stateManager.GetAudio().PlaySound("world_shrink");
		m_screenShake.Trigger(0.5f, 5.0f);
		m_world.FlashBorders(0.3f);

		// Double-shrink may have crushed the snake
		m_world.CheckCollision(window, m_snake);
		if (m_snake.HasLost()) return;
	}

	// Check level complete
	if (m_applesEaten >= m_levelConfig.applesToWin)
	{
		m_stateManager.score += 1000; // level complete bonus
		m_stateManager.GetAudio().PlaySound("level_complete");

		// Star calculation based on self-collisions
		int stars = 1;
		if (m_stateManager.selfCollisions <= m_levelConfig.starThreshold2)
			stars = 2;
		if (m_stateManager.selfCollisions <= m_levelConfig.starThreshold3)
			stars = 3;

		// Update persistent progress
		int idx = m_stateManager.currentLevel - 1;
		if (idx >= 0 && idx < NUM_LEVELS)
		{
			if (m_stateManager.score > m_stateManager.highScores[idx])
				m_stateManager.highScores[idx] = m_stateManager.score;
			if (stars > m_stateManager.starRatings[idx])
				m_stateManager.starRatings[idx] = stars;
		}
		if (m_stateManager.currentLevel >= m_stateManager.highestUnlockedLevel)
			m_stateManager.highestUnlockedLevel = std::min(NUM_LEVELS, m_stateManager.currentLevel + 1);

		m_stateManager.levelComplete = true;
		m_levelCompleteDelay = 0.5f;
	}
}

void PlayState::OnDeath()
{
	m_stateManager.totalDeaths++;
	m_stateManager.levelComplete = false;
	m_stateManager.GetAudio().PlaySound("wall_death");

	// Trigger ink-run death effect
	m_deathInkRunActive = true;
	m_deathInkRunTimer = 0.0f;

	// Spawn ink drip particles at snake head
	float bs = m_snake.GetBlockSize();
	sf::Vector2f headPixel(m_snake.GetPosition().x * bs, m_snake.GetPosition().y * bs);
	m_particles.SpawnInkDrips(headPixel, m_levelConfig.inkTint, 8);

	m_stateManager.SwitchTo(StateType::GameOver);
}

void PlayState::UpdateCombo(bool l_reset)
{
	if (l_reset)
	{
		m_consecutiveApples = 0;
		m_stateManager.comboMultiplier = 1.0f;
		m_stateManager.combo = 0;
		m_comboSoundPlayed = false;
		return;
	}

	m_stateManager.combo = m_consecutiveApples;
	if (m_consecutiveApples >= 5)
		m_stateManager.comboMultiplier = 3.0f;
	else if (m_consecutiveApples >= 4)
		m_stateManager.comboMultiplier = 2.5f;
	else if (m_consecutiveApples >= 3)
		m_stateManager.comboMultiplier = 2.0f;
	else if (m_consecutiveApples >= 2)
		m_stateManager.comboMultiplier = 1.5f;
	else
		m_stateManager.comboMultiplier = 1.0f;

	if (m_stateManager.comboMultiplier >= 3.0f)
	{
		m_hud.FlashCombo();
		if (m_consecutiveApples >= 5 && !m_comboSoundPlayed)
		{
			m_stateManager.GetAudio().PlaySound("combo_3x");
			m_comboSoundPlayed = true;
		}
	}
}

int PlayState::CalculatePoints(int l_base)
{
	return (int)(l_base * m_stateManager.comboMultiplier);
}

float PlayState::GetAppleTimerDuration() const
{
	// Level 10: timer controlled by phase system
	if (m_levelConfig.id == 10)
		return m_levelConfig.appleTimerSec;

	// Level 5 (Famine): progressive timer escalation
	float timer = m_levelConfig.appleTimerSec;
	if (m_applesEaten >= 15) timer = 2.0f;
	else if (m_applesEaten >= 10) timer = 3.0f;
	return timer;
}

void PlayState::RenderPhaseAnnouncement(Window& l_window)
{
	if (!m_announcementFontLoaded)
		return;

	const float totalDuration = 2.0f;
	float elapsed = totalDuration - m_phaseAnnouncementTimer;

	// Fade in 0.3s, hold, fade out 0.5s
	float alpha = 1.0f;
	if (elapsed < 0.3f)
		alpha = elapsed / 0.3f;
	else if (m_phaseAnnouncementTimer < 0.5f)
		alpha = m_phaseAnnouncementTimer / 0.5f;

	sf::Uint8 a = (sf::Uint8)(255 * alpha);
	sf::Vector2u winSize = l_window.GetWindowSize();

	// Semi-transparent dark overlay
	sf::RectangleShape overlay(sf::Vector2f((float)winSize.x, (float)winSize.y));
	overlay.setPosition(0.f, 0.f);
	overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(120 * alpha)));
	l_window.Draw(overlay);

	// Red announcement text, centered
	sf::Text text;
	text.setFont(m_announcementFont);
	text.setString(m_phaseAnnouncementText);
	text.setCharacterSize(48);
	text.setFillColor(sf::Color(220, 30, 30, a));

	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(bounds.left + bounds.width / 2.0f,
				   bounds.top + bounds.height / 2.0f);
	text.setPosition((float)winSize.x / 2.0f, (float)winSize.y / 2.0f);

	l_window.Draw(text);
}
