#include "PlayState.h"
#include "AudioManager.h"
#include <algorithm>

PlayState::PlayState(StateManager& l_stateManager)
	: BaseState(l_stateManager),
	  m_world(l_stateManager.GetWindow(), m_snake),
	  m_hud(l_stateManager.GetWindow().GetWindowSize()),
	  m_elapsedTime(0.0f),
	  m_gameTime(0.0f),
	  m_applesEaten(0),
	  m_consecutiveApples(0),
	  m_paused(false),
	  m_lastShrinkCount(0),
	  m_cheatExtend(false),
	  m_escReleased(true),
	  m_rReleased(true)
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

	m_elapsedTime = 0.0f;
	m_gameTime = 0.0f;
	m_applesEaten = 0;
	m_consecutiveApples = 0;
	m_lastShrinkCount = 0;
	m_paused = false;
	m_cheatExtend = false;
	m_escReleased = true;
	m_rReleased = true;

	m_stateManager.score = 0;
	m_stateManager.applesEaten = 0;
	m_stateManager.combo = 0;
	m_stateManager.comboMultiplier = 1.0f;
	m_stateManager.selfCollisions = 0;
	m_stateManager.levelTime = 0.0f;
	m_stateManager.levelComplete = false;

	m_particles.Clear();
	m_screenShake.Reset(m_stateManager.GetWindow());
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

	float timeStep = 1.0f / speed;

	if (m_elapsedTime >= timeStep)
	{
		Window& window = m_stateManager.GetWindow();

		m_world.Update(window, m_snake);
		m_snake.Tick(window.GetWindowSize());

		// Detect apple eaten by comparing count
		int newApplesEaten = m_world.GetApplesEaten();
		if (newApplesEaten > m_applesEaten)
		{
			m_applesEaten = newApplesEaten;
			OnAppleEaten();
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
			m_particles.SpawnSelfCollisionCut(m_snake.GetLastCutSegments(), m_snake.GetBlockSize());
			m_snake.ClearSelfCollideFlag();
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
}

void PlayState::Render()
{
	Window& window = m_stateManager.GetWindow();

	m_world.Render(window);
	m_snake.Render(window);
	m_particles.Render(window);
	m_hud.Render(window);
}

void PlayState::OnAppleEaten()
{
	m_consecutiveApples++;
	UpdateCombo(false);

	int points = CalculatePoints(100);
	m_stateManager.score += points;
	m_stateManager.applesEaten = m_applesEaten;

	// Audio + visual feedback
	m_stateManager.GetAudio().PlaySound("apple_eat");
	sf::Vector2f applePixelPos(
		m_snake.GetPosition().x * m_snake.GetBlockSize(),
		m_snake.GetPosition().y * m_snake.GetBlockSize());
	m_particles.SpawnAppleBurst(applePixelPos, sf::Color::Green);
	m_particles.SpawnFloatingText("+" + std::to_string(points), applePixelPos,
								  sf::Color(255, 255, 100));

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
		m_stateManager.SwitchTo(StateType::GameOver);
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
		if (m_consecutiveApples == 5) // fire only on the transition to 3x
			m_stateManager.GetAudio().PlaySound("combo_3x");
	}
}

int PlayState::CalculatePoints(int l_base)
{
	return (int)(l_base * m_stateManager.comboMultiplier);
}
