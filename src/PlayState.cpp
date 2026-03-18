#include "PlayState.h"
#include "AudioManager.h"
#include <algorithm>
#include <string>

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
	m_world.RespawnApple(m_snake);
	m_world.SetBorderColor(m_levelConfig.border);
	m_hud.SetLevelColors(m_levelConfig.border, m_levelConfig.background);

	// Apply level-specific configuration
	m_world.SetShrinkInterval(m_levelConfig.shrinkInterval);
	m_world.SetShrinkTimerSec(m_levelConfig.shrinkTimerSec);
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

	// Snake direction (arrow keys)
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && m_snake.GetDirection() != Direction::Down)
		m_snake.SetDirection(Direction::Up);
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && m_snake.GetDirection() != Direction::Up)
		m_snake.SetDirection(Direction::Down);
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && m_snake.GetDirection() != Direction::Left)
		m_snake.SetDirection(Direction::Right);
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && m_snake.GetDirection() != Direction::Right)
		m_snake.SetDirection(Direction::Left);

	// WASD support
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && m_snake.GetDirection() != Direction::Down)
		m_snake.SetDirection(Direction::Up);
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && m_snake.GetDirection() != Direction::Up)
		m_snake.SetDirection(Direction::Down);
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && m_snake.GetDirection() != Direction::Left)
		m_snake.SetDirection(Direction::Right);
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && m_snake.GetDirection() != Direction::Right)
		m_snake.SetDirection(Direction::Left);

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
		m_world.UpdateTimedShrink(l_dt, m_stateManager.GetWindow(), m_snake);
	}

	// Timed apples (Level 5)
	if (m_levelConfig.hasTimedApples && m_levelCompleteDelay < 0.0f)
	{
		m_timedApple.Update(l_dt);

		if (m_timedApple.HasExpired())
		{
			// Penalty: apple missed, world shrinks
			Window& window = m_stateManager.GetWindow();
			m_world.TriggerShrink(window, m_snake);
			m_lastShrinkCount = m_world.GetShrinkCount();
			m_stateManager.GetAudio().PlaySound("apple_miss");
			m_screenShake.Trigger(0.3f, 3.0f);
			m_world.FlashBorders(0.2f);

			// Respawn apple with adjusted timer
			m_world.RespawnApple(m_snake);
			m_timedApple.OnAppleEaten(GetAppleTimerDuration());
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
