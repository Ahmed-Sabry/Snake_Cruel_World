#include "Snake.h"

Snake::Snake()
{
	m_blockSize = 16;
	m_speed = 12;
	//m_dir = Direction::Right;

	InitList(&m_snakeBody);
	m_bodyRect.setSize({ m_blockSize - 1, m_blockSize - 1 });

	Reset();
}

Snake::~Snake()
{
	DestroyList(&m_snakeBody);
}

void Snake::Reset()
{
	DestroyList(&m_snakeBody);

	m_lose = false;
	m_dir = Direction::None;

	/***************************************************/
	/// Note that Range is:
	/// x = 0 : (windowSize.x / BlockSize)
	/// y = 0 : (windowSize.y / BlockSize)
	/// At least you must Initialize head.
	PushList({ 7, 5 }, &m_snakeBody); // Head		/* Must be Initialized */
	//PushList({ 6, 5 }, &m_snakeBody);
	//PushList({ 5, 5 }, &m_snakeBody);
	//PushList({ 4, 5 }, &m_snakeBody);
	//PushList({ 3, 5 }, &m_snakeBody);
	//PushList({ 2, 5 }, &m_snakeBody);	// Tail
	/***************************************************/

	RetrieveList(0, &m_headPos, &m_snakeBody); // Update Initial value for head position
}

void Snake::Move(sf::Vector2u l_windowSize)
{
	for (int i = ListSize(&m_snakeBody) - 1; i > 0; i--)
	{
		RetrieveList(i - 1, &m_pos, &m_snakeBody);
		ReplaceList(i, m_pos, &m_snakeBody);
	}

	RetrieveList(0, &m_pos, &m_snakeBody);

	switch (m_dir)
	{
		case Direction::Up:
			m_pos.y--;
			if (m_pos.y < 0)
			{
				m_pos.y = l_windowSize.y / m_blockSize;
			}
			ReplaceList(0, m_pos, &m_snakeBody);
			break;

		case Direction::Down:
			m_pos.y++;
			if (m_pos.y > (l_windowSize.y / m_blockSize))
			{
				m_pos.y = 0;
			}
			ReplaceList(0, m_pos, &m_snakeBody);
			break;

		case Direction::Right:
			m_pos.x++;
			if (m_pos.x > (l_windowSize.x / m_blockSize))
			{
				m_pos.x = 0;
			}
			ReplaceList(0, m_pos, &m_snakeBody);
			break;

		case Direction::Left:
			m_pos.x--;
			if (m_pos.x < 0)
			{
				m_pos.x = l_windowSize.x / m_blockSize;
			}
			ReplaceList(0, m_pos, &m_snakeBody);
			break;
		default:
			break;
	}

	m_headPos = m_pos; // Update head position
}

void Snake::CheckCollision()
{
	if (ListSize(&m_snakeBody) < 4)
		return;

	for (int i = 3; i < ListSize(&m_snakeBody); i++)
	{
		RetrieveList(i, &m_pos, &m_snakeBody);

		if ((m_headPos.x == m_pos.x) && (m_headPos.y == m_pos.y)) // If snake eat herself
		{
			// Cut
			for (int j = ListSize(&m_snakeBody) - 1; j >= i; j--)
				PopList(&m_snakeBody);
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
	if (ListSize(&m_snakeBody) > 1)
	{
		Position tail, pretail;
		RetrieveList(ListSize(&m_snakeBody) - 1, &tail, &m_snakeBody);
		RetrieveList(ListSize(&m_snakeBody) - 2, &pretail, &m_snakeBody);

		if (tail.x == pretail.x)
		{
			if (tail.y > pretail.y)
				PushList({ tail.x, tail.y - 1 }, &m_snakeBody);

			else
				PushList({ tail.x, tail.y + 1 }, &m_snakeBody);
		}

		else if (tail.y == pretail.y)
		{
			if (tail.x > pretail.x)
				PushList({ tail.x + 1, tail.y }, &m_snakeBody);

			else
				PushList({ tail.x - 1, tail.y }, &m_snakeBody);
		}
	}

	else
	{
		switch (m_dir)
		{
			case Direction::Up:
				PushList({ m_headPos.x, m_headPos.y + 1 }, &m_snakeBody);
				break;

			case Direction::Down:
				PushList({ m_headPos.x, m_headPos.y - 1 }, &m_snakeBody);
				break;

			case Direction::Right:
				PushList({ m_headPos.x - 1, m_headPos.y }, &m_snakeBody);
				break;

			case Direction::Left:
				PushList({ m_headPos.x + 1, m_headPos.y }, &m_snakeBody);
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
	RetrieveList(0, &m_pos, &m_snakeBody);
	m_bodyRect.setPosition(m_pos.x * m_blockSize, m_pos.y * m_blockSize);
	l_window.Draw(m_bodyRect);
	//

	// Draw Body
	m_bodyRect.setFillColor(sf::Color::Magenta);
	for (int i = 1; i < ListSize(&m_snakeBody); i++)
	{
		RetrieveList(i, &m_pos, &m_snakeBody);
		m_bodyRect.setPosition(m_pos.x * m_blockSize, m_pos.y * m_blockSize);
		l_window.Draw(m_bodyRect);
	}
	//
}