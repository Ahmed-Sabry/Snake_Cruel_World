#include "Predator.h"
#include "World.h"
#include "RandomUtils.h"
#include "InkRenderer.h"
#include <cmath>
#include <algorithm>

Predator::Predator()
	: m_direction(Direction::Right),
	  m_blockSize(16.0f),
	  m_speed(8.0f),
	  m_elapsedTime(0.0f),
	  m_mode(PredatorMode::HUNTING_APPLE),
	  m_applesEaten(0),
	  m_huntTimer(0.0f),
	  m_justAteApple(false),
	  m_justStartedHunting(false),
	  m_modeTransitionTimer(0.0f)
{
}

Position Predator::FindSpawnPosition(const Snake& l_snake, const World& l_world) const
{
	float bs = m_blockSize;
	int xMin = (int)(l_world.GetEffectiveThickness(3) / bs) + 1;
	int xMax = (int)(l_world.GetMaxX() - l_world.GetEffectiveThickness(1) / bs) - 2;
	int yMin = (int)((l_world.GetEffectiveThickness(0) + l_world.GetTopOffset()) / bs) + 1;
	int yMax = (int)(l_world.GetMaxY() - l_world.GetEffectiveThickness(2) / bs) - 2;

	Position playerHead = l_snake.GetPosition();

	// If playable area is too narrow for 5 trailing segments, use center fallback
	if (xMin + 4 > xMax)
		return { (xMin + xMax) / 2, (yMin + yMax) / 2 };

	// Seed best from a legal head position within the valid lane
	Position best = { RandomInt(xMin + 4, xMax), RandomInt(yMin, yMax) };
	int bestDist = std::abs(best.x - playerHead.x) + std::abs(best.y - playerHead.y);

	for (int attempt = 0; attempt < 30; attempt++)
	{
		int x = RandomInt(xMin + 4, xMax); // +4 to leave room for body trailing left
		int y = RandomInt(yMin, yMax);

		int dist = std::abs(x - playerHead.x) + std::abs(y - playerHead.y);
		if (dist >= 10)
			return { x, y };

		if (dist > bestDist)
		{
			bestDist = dist;
			best = { x, y };
		}
	}

	return best;
}

void Predator::Reset(float l_blockSize, const Snake& l_snake, const World& l_world)
{
	m_blockSize = l_blockSize;
	m_body.clear();
	m_speed = 8.0f;
	m_elapsedTime = 0.0f;
	m_mode = PredatorMode::HUNTING_APPLE;
	m_applesEaten = 0;
	m_huntTimer = 0.0f;
	m_justAteApple = false;
	m_justStartedHunting = false;
	m_modeTransitionTimer = 0.0f;
	m_direction = Direction::Right;

	m_bodyRect.setSize({ m_blockSize - 2, m_blockSize - 2 });
	m_eyeShape.setRadius(m_blockSize * 0.125f);
	m_eyeShape.setFillColor(sf::Color::White);

	// Spawn head and 4 trailing body segments to the left
	Position head = FindSpawnPosition(l_snake, l_world);
	for (int i = 0; i < 5; i++)
		m_body.push_back({ head.x - i, head.y });
}

bool Predator::IsWall(const Position& l_pos, const World& l_world) const
{
	float bs = m_blockSize;
	float xLeft = l_world.GetEffectiveThickness(3) / bs;
	float xRight = l_world.GetMaxX() - l_world.GetEffectiveThickness(1) / bs - 1;
	float yTop = (l_world.GetEffectiveThickness(0) + l_world.GetTopOffset()) / bs;
	float yBottom = l_world.GetMaxY() - l_world.GetEffectiveThickness(2) / bs - 1;

	return l_pos.x < (int)xLeft || l_pos.x > (int)xRight ||
		   l_pos.y < (int)yTop || l_pos.y > (int)yBottom;
}

bool Predator::IsOwnBody(const Position& l_pos) const
{
	// Skip head (index 0) and tail (last, will move away)
	for (size_t i = 1; i + 1 < m_body.size(); i++)
	{
		if (m_body[i].x == l_pos.x && m_body[i].y == l_pos.y)
			return true;
	}
	return false;
}

Direction Predator::ChooseDirection(const Position& l_target, const World& l_world,
									const Snake& l_snake) const
{
	if (m_body.empty()) return Direction::Right;

	Position head = m_body[0];

	struct Candidate
	{
		Direction dir;
		Position pos;
		int score;
		bool blocked;
	};

	Candidate candidates[4] = {
		{ Direction::Up,    { head.x, head.y - 1 }, 0, false },
		{ Direction::Down,  { head.x, head.y + 1 }, 0, false },
		{ Direction::Left,  { head.x - 1, head.y }, 0, false },
		{ Direction::Right, { head.x + 1, head.y }, 0, false }
	};

	// Opposite direction map for 180-degree check
	Direction opposite = Direction::None;
	switch (m_direction)
	{
		case Direction::Up:    opposite = Direction::Down;  break;
		case Direction::Down:  opposite = Direction::Up;    break;
		case Direction::Left:  opposite = Direction::Right; break;
		case Direction::Right: opposite = Direction::Left;  break;
		default: break;
	}

	const auto& playerBody = l_snake.GetBody();

	for (int i = 0; i < 4; i++)
	{
		auto& c = candidates[i];

		// Reject 180-degree reversal
		if (c.dir == opposite)
		{
			c.blocked = true;
			c.score = 100000;
			continue;
		}

		// Reject walls
		if (IsWall(c.pos, l_world))
		{
			c.blocked = true;
			c.score = 50000;
			continue;
		}

		// Reject own body
		if (IsOwnBody(c.pos))
		{
			c.blocked = true;
			c.score = 40000;
			continue;
		}

		// Base score: Manhattan distance to target
		c.score = std::abs(c.pos.x - l_target.x) + std::abs(c.pos.y - l_target.y);

		// Soft penalty for overlapping player body (avoid but don't reject).
		// Skip playerBody[0] (head) when hunting the player — that's the target.
		for (size_t s = (m_mode == PredatorMode::HUNTING_PLAYER) ? 1 : 0; s < playerBody.size(); s++)
		{
			if (playerBody[s].x == c.pos.x && playerBody[s].y == c.pos.y)
			{
				c.score += 1000;
				break;
			}
		}
	}

	// Pick lowest-score unblocked candidate
	int bestIdx = -1;
	for (int i = 0; i < 4; i++)
	{
		if (candidates[i].blocked) continue;
		if (bestIdx < 0 || candidates[i].score < candidates[bestIdx].score)
			bestIdx = i;
	}

	// All directions blocked — pause (don't move)
	if (bestIdx < 0)
		return Direction::None;

	return candidates[bestIdx].dir;
}

void Predator::Move()
{
	if (m_body.empty()) return;

	// Direction::None means all directions blocked — pause in place
	if (m_direction == Direction::None) return;

	// Shift body segments backward (tail follows head)
	for (int i = (int)m_body.size() - 1; i > 0; i--)
		m_body[i] = m_body[i - 1];

	// Move head
	switch (m_direction)
	{
		case Direction::Up:    m_body[0].y--; break;
		case Direction::Down:  m_body[0].y++; break;
		case Direction::Left:  m_body[0].x--; break;
		case Direction::Right: m_body[0].x++; break;
		default: break;
	}
}

void Predator::Grow()
{
	if (m_body.empty()) return;
	// Duplicate tail segment
	m_body.push_back(m_body.back());
}

void Predator::Update(float l_dt, const World& l_world, const Snake& l_snake)
{
	if (m_body.empty()) return;

	m_elapsedTime += l_dt;

	// Mode transition visual timer (cosmetic, fine to use l_dt)
	if (m_modeTransitionTimer > 0.0f)
		m_modeTransitionTimer -= l_dt;

	float timeStep = (m_speed > 0.0f) ? (1.0f / m_speed) : 1.0f;

	while (m_elapsedTime >= timeStep && timeStep > 0.0f)
	{
		// Hunt timer countdown (per-tick so mode flips at the correct step)
		if (m_mode == PredatorMode::HUNTING_PLAYER)
		{
			m_huntTimer -= timeStep;
			if (m_huntTimer <= 0.0f)
			{
				m_mode = PredatorMode::HUNTING_APPLE;
				m_huntTimer = 0.0f;
				m_modeTransitionTimer = 1.0f;
			}
		}

		// Determine target
		Position target;
		if (m_mode == PredatorMode::HUNTING_PLAYER)
		{
			target = l_snake.GetPosition();
		}
		else
		{
			sf::Vector2f applePos = l_world.GetApplePos();
			target = { (int)applePos.x, (int)applePos.y };
		}

		// Choose direction and move
		m_direction = ChooseDirection(target, l_world, l_snake);
		Move();

		// Check if predator ate the apple
		if (m_mode == PredatorMode::HUNTING_APPLE)
		{
			sf::Vector2f applePos = l_world.GetApplePos();
			if (m_body[0].x == (int)applePos.x && m_body[0].y == (int)applePos.y)
			{
				m_applesEaten++;
				m_justAteApple = true;
				Grow();
				m_speed += 1.0f;

				// After every 3 apples eaten, switch to hunting player
				if (m_applesEaten % 3 == 0)
				{
					m_mode = PredatorMode::HUNTING_PLAYER;
					m_huntTimer = 10.0f;
					m_justStartedHunting = true;
					m_modeTransitionTimer = 1.0f;
				}

				// Stop simulation — let PlayState handle shrink/respawn
				// before we continue with updated world state
				m_elapsedTime -= timeStep;
				break;
			}
		}

		m_elapsedTime -= timeStep;
	}
}

void Predator::Render(Window& l_window, float l_blockSize)
{
	RenderTo(l_window.GetRenderWindow(), l_blockSize);
}

void Predator::RenderTo(sf::RenderTarget& target, float l_blockSize)
{
	if (m_body.empty()) return;

	// Ink-toned colors based on mode
	sf::Color headColor, bodyColor;
	if (m_mode == PredatorMode::HUNTING_APPLE)
	{
		headColor = sf::Color(40, 70, 120);
		bodyColor = sf::Color(30, 55, 100);
	}
	else
	{
		headColor = sf::Color(160, 40, 30);
		bodyColor = sf::Color(130, 25, 20);
	}

	// Pulse during mode transition
	if (m_modeTransitionTimer > 0.0f)
	{
		float pulse = std::sin(m_modeTransitionTimer * 15.0f);
		if (pulse > 0.0f)
		{
			if (m_mode == PredatorMode::HUNTING_PLAYER)
			{
				headColor = sf::Color(40, 70, 120);
				bodyColor = sf::Color(30, 55, 100);
			}
			else
			{
				headColor = sf::Color(160, 40, 30);
				bodyColor = sf::Color(130, 25, 20);
			}
		}
	}

	// Draw body segments with stipple pattern (different from snake's hatch)
	sf::Color outlineColor(30, 25, 20, 180);
	for (int i = (int)m_body.size() - 1; i >= 0; i--)
	{
		sf::Color color = (i == 0) ? headColor : bodyColor;
		float px = m_body[i].x * l_blockSize + 1.0f;
		float py = m_body[i].y * l_blockSize + 1.0f;
		float segSize = l_blockSize - 2.0f;

		// Wobbly rect with stipple dots inside (drawn as the fill)
		InkRenderer::DrawWobblyRect(target, px, py, segSize, segSize,
									color, outlineColor, 1.0f,
									0.2f, (unsigned int)(i * 41 + 300));

		// Stipple dots overlay (differentiates from player snake's hatch)
		int dotCount = 4;
		for (int d = 0; d < dotCount; d++)
		{
			unsigned int h = InkRenderer::Hash((unsigned int)(i * 41 + 300), (unsigned int)d);
			float dx = (float)(h % (int)segSize);
			float dy = (float)((h >> 8) % (int)segSize);
			sf::CircleShape dot(0.8f);
			dot.setPosition(px + dx, py + dy);
			dot.setFillColor(sf::Color(outlineColor.r, outlineColor.g, outlineColor.b, 100));
			target.draw(dot);
		}
	}

	// Draw eyes with tracking pupils
	{
		float cx = m_body[0].x * l_blockSize + l_blockSize / 2.0f;
		float cy = m_body[0].y * l_blockSize + l_blockSize / 2.0f;

		float ex1 = 0, ey1 = 0, ex2 = 0, ey2 = 0;
		float fwd = l_blockSize * 0.1875f;
		float side = l_blockSize * 0.1875f;

		switch (m_direction)
		{
			case Direction::Up:
				ex1 = cx - side; ey1 = cy - fwd;
				ex2 = cx + side; ey2 = cy - fwd;
				break;
			case Direction::Down:
				ex1 = cx - side; ey1 = cy + fwd;
				ex2 = cx + side; ey2 = cy + fwd;
				break;
			case Direction::Left:
				ex1 = cx - fwd; ey1 = cy - side;
				ex2 = cx - fwd; ey2 = cy + side;
				break;
			case Direction::Right:
				ex1 = cx + fwd; ey1 = cy - side;
				ex2 = cx + fwd; ey2 = cy + side;
				break;
			default:
				ex1 = cx - side; ey1 = cy - fwd;
				ex2 = cx + side; ey2 = cy - fwd;
				break;
		}

		// Eye whites (larger circles)
		float eyeR = l_blockSize * 0.15f;
		m_eyeShape.setRadius(eyeR);
		m_eyeShape.setFillColor(sf::Color(240, 240, 230));
		m_eyeShape.setPosition(ex1 - eyeR, ey1 - eyeR);
		target.draw(m_eyeShape);
		m_eyeShape.setPosition(ex2 - eyeR, ey2 - eyeR);
		target.draw(m_eyeShape);

		// Pupils — offset toward direction of movement for tracking look
		float pupilR = l_blockSize * 0.07f;
		float pupilOffset = eyeR * 0.3f;
		float pdx = 0, pdy = 0;
		switch (m_direction)
		{
			case Direction::Up:    pdy = -pupilOffset; break;
			case Direction::Down:  pdy = pupilOffset;  break;
			case Direction::Left:  pdx = -pupilOffset; break;
			case Direction::Right: pdx = pupilOffset;  break;
			default: break;
		}

		sf::CircleShape pupil(pupilR);
		pupil.setFillColor(sf::Color(20, 15, 10));
		pupil.setPosition(ex1 - pupilR + pdx, ey1 - pupilR + pdy);
		target.draw(pupil);
		pupil.setPosition(ex2 - pupilR + pdx, ey2 - pupilR + pdy);
		target.draw(pupil);
	}
}

bool Predator::HitPlayer(const Position& l_playerHead) const
{
	for (const auto& seg : m_body)
	{
		if (seg.x == l_playerHead.x && seg.y == l_playerHead.y)
			return true;
	}
	return false;
}

PredatorMode Predator::GetMode() const
{
	return m_mode;
}

int Predator::GetApplesEaten() const
{
	return m_applesEaten;
}

bool Predator::JustAteApple()
{
	if (m_justAteApple)
	{
		m_justAteApple = false;
		return true;
	}
	return false;
}

bool Predator::JustStartedHunting()
{
	if (m_justStartedHunting)
	{
		m_justStartedHunting = false;
		return true;
	}
	return false;
}

void Predator::OnPlayerAteApple()
{
	// Predator retargets automatically on next tick since
	// ChooseDirection reads the current apple position each tick.
	// Nothing explicit needed here.
}

const std::vector<Position>& Predator::GetBody() const
{
	return m_body;
}
