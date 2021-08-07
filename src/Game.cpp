#include "Game.h"

Game::Game() :
	m_window({ 1366, 768 }, "Hello Cruel World"),
	m_world(m_window, m_snake)
{
	// {1366, 768}
	m_window.SetBackground(sf::Color(30, 15, 20, 255));
	m_elapsedTime = 0.0f;
	m_world.Borders(m_window);
	m_world.RespawnApple(m_snake);
}

Game::~Game()
{
}

void Game::Update()
{
	m_window.Update();

	float timeStep = 1.0f / m_snake.GetSpeed();

	if (m_elapsedTime >= timeStep)
	{
		m_world.Update(m_window, m_snake);
		m_snake.Tick(m_window.GetWindowSize());

		//std::cout << "Snake.x = " << m_snake.GetPosition().x << ", Snake.y = " << m_snake.GetPosition().y << "\n";
		//std::cout << "Apple.x = " << m_world.GetApplePos().x << ", Apple.y = " << m_world.GetApplePos().y << "\n\n";

		if (m_snake.HasLost())
			m_snake.Reset();

		// Cheat code grow up extend the snake
		if (e == true)
		{
			m_snake.Extend();
			e = false;
		}

		m_elapsedTime -= timeStep;
	}
}

void Game::HandleInput()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && (m_snake.GetDirection() != Direction::Down))
		m_snake.SetDirection(Direction::Up);

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && (m_snake.GetDirection() != Direction::Up))
		m_snake.SetDirection(Direction::Down);

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && (m_snake.GetDirection() != Direction::Left))
		m_snake.SetDirection(Direction::Right);

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && (m_snake.GetDirection() != Direction::Right))
		m_snake.SetDirection(Direction::Left);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		e = true;
}

void Game::Render()
{
	m_window.Clear();

	m_world.Render(m_window);
	m_snake.Render(m_window);

	m_window.Display();
}

void Game::RestartClock()
{
	m_elapsedTime += m_clock.restart().asSeconds();
}
