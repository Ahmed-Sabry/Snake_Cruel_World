#include "Snake.h"

Snake::Snake()
{
	m_blockSize = 16;
	m_speed = 12;

	m_bodyRect.setSize({ m_blockSize - 1, m_blockSize - 1 });

	Reset();
}

Snake::~Snake()
{
}

void Snake::Reset()
{
	m_snakeBody.clear();

	m_lose = false;
	m_selfCollided = false;
	m_dir = Direction::None;

	/***************************************************/
	/// Note that Range is:
	/// x = 0 : (windowSize.x / BlockSize)
	/// y = 0 : (windowSize.y / BlockSize)
	/// At least you must Initialize head.
	m_snakeBody.push_back({ 7, 5 }); // Head		/* Must be Initialized */
	/***************************************************/

	m_headPos = m_snakeBody[0]; // Update Initial value for head position
}

void Snake::Move(sf::Vector2u l_windowSize)
{
	for (int i = m_snakeBody.size() - 1; i > 0; i--)
	{
		m_snakeBody[i] = m_snakeBody[i - 1];
	}

	switch (m_dir)
	{
		case Direction::Up:
			m_snakeBody[0].y--;
			if (m_snakeBody[0].y < 0)
			{
				m_snakeBody[0].y = l_windowSize.y / m_blockSize;
			}
			break;

		case Direction::Down:
			m_snakeBody[0].y++;
			if (m_snakeBody[0].y > (int)(l_windowSize.y / m_blockSize))
			{
				m_snakeBody[0].y = 0;
			}
			break;

		case Direction::Right:
			m_snakeBody[0].x++;
			if (m_snakeBody[0].x > (int)(l_windowSize.x / m_blockSize))
			{
				m_snakeBody[0].x = 0;
			}
			break;

		case Direction::Left:
			m_snakeBody[0].x--;
			if (m_snakeBody[0].x < 0)
			{
				m_snakeBody[0].x = l_windowSize.x / m_blockSize;
			}
			break;
		default:
			break;
	}

	m_headPos = m_snakeBody[0]; // Update head position
}

void Snake::CheckCollision()
{
	if ((int)m_snakeBody.size() < 4)
		return;

	for (int i = 3; i < (int)m_snakeBody.size(); i++)
	{
		if ((m_headPos.x == m_snakeBody[i].x) && (m_headPos.y == m_snakeBody[i].y)) // If snake eat herself
		{
			// Cut
			m_snakeBody.erase(m_snakeBody.begin() + i, m_snakeBody.end());
			m_selfCollided = true;
		}
	}
}

void Snake::Tick(sf::Vector2u l_windowSize)
{
	Move(l_windowSize);
	CheckCollision();
}

void Snake::Extend()
{
	if ((int)m_snakeBody.size() > 1)
	{
		Position tail = m_snakeBody.back();
		Position pretail = m_snakeBody[m_snakeBody.size() - 2];

		if (tail.x == pretail.x)
		{
			if (tail.y > pretail.y)
				m_snakeBody.push_back({ tail.x, tail.y - 1 });
			else
				m_snakeBody.push_back({ tail.x, tail.y + 1 });
		}
		else if (tail.y == pretail.y)
		{
			if (tail.x > pretail.x)
				m_snakeBody.push_back({ tail.x + 1, tail.y });
			else
				m_snakeBody.push_back({ tail.x - 1, tail.y });
		}
	}
	else
	{
		switch (m_dir)
		{
			case Direction::Up:
				m_snakeBody.push_back({ m_headPos.x, m_headPos.y + 1 });
				break;

			case Direction::Down:
				m_snakeBody.push_back({ m_headPos.x, m_headPos.y - 1 });
				break;

			case Direction::Right:
				m_snakeBody.push_back({ m_headPos.x - 1, m_headPos.y });
				break;

			case Direction::Left:
				m_snakeBody.push_back({ m_headPos.x + 1, m_headPos.y });
				break;
			default:
				break;
		}
	}
}

void Snake::Render(Window& l_window)
{
	// Draw Head
	m_bodyRect.setFillColor(sf::Color::Red);
	m_bodyRect.setPosition(m_snakeBody[0].x * m_blockSize, m_snakeBody[0].y * m_blockSize);
	l_window.Draw(m_bodyRect);

	// Draw Body
	m_bodyRect.setFillColor(sf::Color::Magenta);
	for (int i = 1; i < (int)m_snakeBody.size(); i++)
	{
		m_bodyRect.setPosition(m_snakeBody[i].x * m_blockSize, m_snakeBody[i].y * m_blockSize);
		l_window.Draw(m_bodyRect);
	}
}
