#include "PlayState.h"
#include "AudioManager.h"
#include <algorithm>
#include <string>
#include <cmath>

PlayState::PlayState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_world(l_stateManager.GetWindow(), m_snake),
	  m_hud(l_stateManager.GetWindow().GetWindowSize()),
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
	  m_levelCompleteDelay(-1.0f)
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
		m_quicksand.Reset(m_world.GetMaxX(), m_world.GetMaxY(),
						  m_world.GetBorderThickness(), m_snake.GetBlockSize(),
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
			float centerX = (m_world.GetBorderThickness() / bs +
							 m_world.GetMaxX() - m_world.GetBorderThickness() / bs - 1) / 2.0f;
			float centerY = ((m_world.GetBorderThickness() + m_world.GetTopOffset()) / bs +
							 m_world.GetMaxY() - m_world.GetBorderThickness() / bs - 1) / 2.0f;
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

		// Check death
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
				 m_levelConfig.name, m_gameTime, l_dt);

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
		m_quicksand.Update(l_dt, m_world.GetMaxX(), m_world.GetMaxY(),
						   m_world.GetBorderThickness(), m_snake.GetBlockSize(),
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

	m_world.Render(window);

	if (m_levelConfig.hasQuicksand)
		m_quicksand.Render(window, m_snake.GetBlockSize());

	if (m_levelConfig.hasTimedApples)
	{
		sf::Vector2f applePixelPos(
			m_world.GetApplePos().x * m_snake.GetBlockSize(),
			m_world.GetApplePos().y * m_snake.GetBlockSize());
		m_timedApple.Render(window, applePixelPos, m_snake.GetBlockSize() / 2.0f);
	}

	if (m_levelConfig.hasMirrorGhost)
	{
		float bs = m_snake.GetBlockSize();
		int bMinX = (int)(m_world.GetBorderThickness() / bs);
		int bMaxX = (int)(m_world.GetMaxX() - m_world.GetBorderThickness() / bs - 1);
		int bMinY = (int)((m_world.GetBorderThickness() + m_world.GetTopOffset()) / bs);
		int bMaxY = (int)(m_world.GetMaxY() - m_world.GetBorderThickness() / bs - 1);
		m_mirrorGhost.Render(window, bs, bMinX, bMaxX, bMinY, bMaxY);
	}

	// Poison apple rendering + snake flash
	if (m_levelConfig.hasPoisonApples)
	{
		m_poisonApple.Render(window, m_snake.GetBlockSize());

		// In Phase 2, make the real apple pulse too
		if (m_poisonApple.GetRealApplesEaten() >= 8)
		{
			float pulseRadius = (m_snake.GetBlockSize() / 2.0f) + std::sin(m_gameTime * 4.0f) * 1.0f;
			sf::CircleShape realPulse;
			realPulse.setRadius(pulseRadius);
			float baseRadius = m_snake.GetBlockSize() / 2.0f;
			realPulse.setOrigin(pulseRadius - baseRadius, pulseRadius - baseRadius);
			realPulse.setFillColor(m_levelConfig.apple);
			realPulse.setPosition(m_world.GetApplePos().x * m_snake.GetBlockSize(),
								  m_world.GetApplePos().y * m_snake.GetBlockSize());
			window.Draw(realPulse);
		}

		// Snake flash when poisoned, restore when not
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

	m_snake.Render(window);
	m_particles.Render(window);

	if (m_levelConfig.hasBlackouts)
		m_blackout.Render(window, m_snake.GetBlockSize());

	m_hud.Render(window);
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
								  sf::Color(255, 255, 100));

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

	// Timed apples: reset timer with adjusted duration
	if (m_levelConfig.hasTimedApples)
		m_timedApple.OnAppleEaten(GetAppleTimerDuration());

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
	float timer = m_levelConfig.appleTimerSec;
	if (m_applesEaten >= 15) timer = 2.0f;
	else if (m_applesEaten >= 10) timer = 3.0f;
	return timer;
}
