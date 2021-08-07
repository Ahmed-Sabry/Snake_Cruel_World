#include "World.h"

int Score;

World::World(Window& l_window, Snake& l_snake)
{

	Reset(l_window, l_snake);

	m_appleRaduis = l_snake.GetBlockSize() / 2;
	m_apple.setRadius(m_appleRaduis);
	m_apple.setFillColor(sf::Color::Green);

	m_maxX = (l_window.GetWindowSize().x / l_snake.GetBlockSize());
	m_maxY = (l_window.GetWindowSize().y / l_snake.GetBlockSize());

	srand(time(nullptr));
}

World::~World()
{
}

void World::Reset(Window& l_window, Snake& l_snake)
{
	m_borderThinkness = l_snake.GetBlockSize();
	Borders(l_window);
	m_count = 0;
}

void World::Borders(Window& l_window)
{

	for (int i = 0; i < 4; i++)
	{
		m_borders[i].setFillColor(sf::Color(200, 100, 50, 255));

		if (i % 2)																			// if odd
			m_borders[i].setSize({ m_borderThinkness, (float)l_window.GetWindowSize().y }); // Right & Left
		else																				// if even
			m_borders[i].setSize({ (float)l_window.GetWindowSize().x, m_borderThinkness }); // Top & Down
	}

	m_borders[0].setPosition({ 0, 0 });
	m_borders[2].setPosition({ 0, (float)l_window.GetWindowSize().y - m_borderThinkness });
	m_borders[1].setPosition({ (float)l_window.GetWindowSize().x - m_borderThinkness, 0 });
	m_borders[3].setPosition({ 0, 0 });
}

void World::NarrowWorld(Window& l_window, Snake& l_snake)
{
	m_borderThinkness += l_snake.GetBlockSize();
	Borders(l_window);
	Score += 10; // Increase Score when the world get smaller
}

void World::RespawnApple(Snake& l_snake)
{
	float x = Random(m_borderThinkness / l_snake.GetBlockSize(), m_maxX - (m_borderThinkness / l_snake.GetBlockSize()) - 2 * (m_appleRaduis / l_snake.GetBlockSize()));
	float y = Random(m_borderThinkness / l_snake.GetBlockSize(), m_maxY - (m_borderThinkness / l_snake.GetBlockSize()) - 2 * (m_appleRaduis / l_snake.GetBlockSize()));

	m_apple.setPosition(sf::Vector2f(x * l_snake.GetBlockSize(), y * l_snake.GetBlockSize()));
	m_applePos = { x, y }; // Store apple position
}

void World::CheckCollision(Window& l_window, Snake& l_snake)
{
	float xLeft = m_borderThinkness / l_snake.GetBlockSize();
	float xRight = m_maxX - (m_borderThinkness / l_snake.GetBlockSize()) - 1;
	float yLeft = m_borderThinkness / l_snake.GetBlockSize();
	float yRight = m_maxY - (m_borderThinkness / l_snake.GetBlockSize()) - 1;

	if ((l_snake.GetPosition().x < xLeft) || (l_snake.GetPosition().x > xRight) || (l_snake.GetPosition().y < yLeft) || (l_snake.GetPosition().y > yRight))
	{
		l_snake.LoseStatus(true);
		Reset(l_window, l_snake);
	}
}

void World::Update(Window& l_window, Snake& l_snake)
{
	if (l_snake.GetPosition().x == m_applePos.x && l_snake.GetPosition().y == m_applePos.y)
	{
		if (++m_count == 4)
		{
			NarrowWorld(l_window, l_snake);
			m_count = 0;
		}

		l_snake.Extend();
		RespawnApple(l_snake);
	}

	CheckCollision(l_window, l_snake);
}

void World::Render(Window& l_window)
{
	for (int i = 0; i < 4; i++)
		l_window.Draw(m_borders[i]);

	l_window.Draw(m_apple);
}