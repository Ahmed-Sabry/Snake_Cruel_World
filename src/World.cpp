#include "World.h"
#include "InkRenderer.h"
#include <algorithm>
#include <cmath>

World::World(Window& l_window, Snake& l_snake)
{
	m_topOffset = 0.f;
	m_flashTimer = 0.0f;
	m_levelId = 1;
	m_normalBorderColor = sf::Color(200, 100, 50);
	m_bossArenaEnabled = false;
	m_disableShrinkForBossArena = false;
	m_allowBossSpecificSpawns = false;
	m_useInkStyle = false;
	m_corruption = 0.05f;
	m_inkTint = sf::Color(60, 50, 45);
	m_accentColor = sf::Color(180, 60, 50);
	m_appleColor = sf::Color::Green;
	m_appleSeed = 42;
	ResetBorderOffsets();
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
	m_borderThickness = l_snake.GetBlockSize();
	ResetBorderOffsets();
	ClearBossArenaMode();
	Borders(l_window);
	m_count = 0;
	m_totalApplesEaten = 0;
	m_shrinkCount = 0;
	m_shrinkInterval = 4;
	m_shrinkTimerSec = 0.0f;
	m_shrinkTimerAccum = 0.0f;
}

void World::SetBorderColor(sf::Color l_color)
{
	m_normalBorderColor = l_color;
	for (int i = 0; i < 4; i++)
		m_borders[i].setFillColor(l_color);
}

void World::FlashBorders(float l_duration)
{
	if (l_duration <= 0.0f)
		return;

	m_flashTimer = l_duration;
	for (int i = 0; i < 4; i++)
		m_borders[i].setFillColor(sf::Color::Red);
}

void World::UpdateFlash(float l_dt)
{
	if (m_flashTimer <= 0.0f)
		return;

	m_flashTimer -= l_dt;
	if (m_flashTimer <= 0.0f)
	{
		m_flashTimer = 0.0f;
		for (int i = 0; i < 4; i++)
			m_borders[i].setFillColor(m_normalBorderColor);
	}
}

void World::SetTopOffset(float l_offset)
{
	m_topOffset = l_offset;
}

void World::Borders(Window& l_window)
{
	float winWidth = (float)l_window.GetWindowSize().x;
	float winHeight = (float)l_window.GetWindowSize().y;

	float effTop = GetEffectiveThickness(0);
	float effRight = GetEffectiveThickness(1);
	float effBottom = GetEffectiveThickness(2);
	float effLeft = GetEffectiveThickness(3);

	for (int i = 0; i < 4; i++)
		m_borders[i].setFillColor(m_normalBorderColor);

	// Top
	m_borders[0].setSize({ winWidth, effTop });
	m_borders[0].setPosition({ 0, m_topOffset });
	// Right
	m_borders[1].setSize({ effRight, winHeight - m_topOffset });
	m_borders[1].setPosition({ winWidth - effRight, m_topOffset });
	// Bottom
	m_borders[2].setSize({ winWidth, effBottom });
	m_borders[2].setPosition({ 0, winHeight - effBottom });
	// Left
	m_borders[3].setSize({ effLeft, winHeight - m_topOffset });
	m_borders[3].setPosition({ 0, m_topOffset });
}

void World::NarrowWorld(Window& l_window, Snake& l_snake)
{
	m_borderThickness += l_snake.GetBlockSize();
	Borders(l_window);
	m_shrinkCount++;

	// If apple is now inside a wall, respawn it within new bounds
	float bs = l_snake.GetBlockSize();
	float xLeft = GetEffectiveThickness(3) / bs;
	float xRight = m_maxX - GetEffectiveThickness(1) / bs - 2 * (m_appleRaduis / bs);
	float yTop = (GetEffectiveThickness(0) + m_topOffset) / bs;
	float yBottom = m_maxY - GetEffectiveThickness(2) / bs - 2 * (m_appleRaduis / bs);

	if (m_applePos.x < xLeft || m_applePos.x > xRight ||
		m_applePos.y < yTop || m_applePos.y > yBottom)
	{
		RespawnApple(l_snake);
	}
}

void World::RespawnApple(Snake& l_snake)
{
	float bs = l_snake.GetBlockSize();
	int xMin = (int)(GetEffectiveThickness(3) / bs);
	int xMax = (int)(m_maxX - GetEffectiveThickness(1) / bs - 2 * (m_appleRaduis / bs));
	int yMin = (int)((GetEffectiveThickness(0) + m_topOffset) / bs);
	int yMax = (int)(m_maxY - GetEffectiveThickness(2) / bs - 2 * (m_appleRaduis / bs));

	// If eating this apple will trigger a shrink, add 1-block inward margin
	// so the snake won't be crushed by the border moving inward
	if (!(m_bossArenaEnabled && m_disableShrinkForBossArena) &&
		m_shrinkInterval > 0 && m_count == m_shrinkInterval - 1)
	{
		xMin += 1;
		xMax -= 1;
		yMin += 1;
		yMax -= 1;
	}

	// Guard against invalid bounds after excessive shrinking
	if (xMin > xMax) xMin = xMax;
	if (yMin > yMax) yMin = yMax;

	float x = Random(xMin, xMax);
	float y = Random(yMin, yMax);

	// Phantom rule: wall bias — 35% chance in levels 6+ to spawn in outer quarter
	if (m_levelId >= 6 && xMax > xMin && yMax > yMin && (rand() % 100) < 35)
	{
		int wall = rand() % 4;
		int quarterX = std::max(1, (xMax - xMin) / 4);
		int quarterY = std::max(1, (yMax - yMin) / 4);
		switch (wall)
		{
			case 0: y = (float)(yMin + rand() % quarterY); break;
			case 1: x = (float)(xMax - rand() % quarterX); break;
			case 2: y = (float)(yMax - rand() % quarterY); break;
			case 3: x = (float)(xMin + rand() % quarterX); break;
			default: break;
		}
	}

	m_apple.setPosition(sf::Vector2f(x * bs, y * bs));
	m_applePos = { x, y };
	m_appleSeed = (unsigned int)(x * 31 + y * 97 + m_totalApplesEaten * 1013);
}

void World::CheckCollision(Window& l_window, Snake& l_snake)
{
	float bs = l_snake.GetBlockSize();
	float xLeft = GetEffectiveThickness(3) / bs;
	float xRight = m_maxX - GetEffectiveThickness(1) / bs - 1;
	float yTop = (GetEffectiveThickness(0) + m_topOffset) / bs;
	float yBottom = m_maxY - GetEffectiveThickness(2) / bs - 1;

	if ((l_snake.GetPosition().x < xLeft) || (l_snake.GetPosition().x > xRight) ||
		(l_snake.GetPosition().y < yTop) || (l_snake.GetPosition().y > yBottom))
	{
		l_snake.LoseStatus(true);
		Reset(l_window, l_snake);
	}
}

void World::Update(Window& l_window, Snake& l_snake)
{
	if (l_snake.GetPosition().x == m_applePos.x && l_snake.GetPosition().y == m_applePos.y)
	{
		if (m_shrinkInterval > 0)
		{
			if (++m_count >= m_shrinkInterval)
			{
				TriggerShrink(l_window, l_snake);
				m_count = 0;
			}
		}

		m_totalApplesEaten++;
		l_snake.Extend();
		RespawnApple(l_snake);
	}

	CheckCollision(l_window, l_snake);
}

void World::SetShrinkInterval(int l_interval)
{
	m_shrinkInterval = l_interval;
}

void World::SetShrinkTimerSec(float l_sec)
{
	m_shrinkTimerSec = l_sec;
}

void World::UpdateTimedShrink(float l_dt, Window& l_window, Snake& l_snake)
{
	if (m_shrinkTimerSec <= 0.0f) return;
	if (m_bossArenaEnabled && m_disableShrinkForBossArena) return;
	m_shrinkTimerAccum += l_dt;
	while (m_shrinkTimerAccum >= m_shrinkTimerSec)
	{
		TriggerShrink(l_window, l_snake);
		m_shrinkTimerAccum -= m_shrinkTimerSec;
	}
}

void World::TriggerShrink(Window& l_window, Snake& l_snake)
{
	if (m_bossArenaEnabled && m_disableShrinkForBossArena)
		return;
	NarrowWorld(l_window, l_snake);
}

void World::SetBossArenaMode(const BossArenaRequirements& l_requirements, float l_blockSize)
{
	m_bossArenaEnabled = l_requirements.usesBossArena;
	m_disableShrinkForBossArena = l_requirements.disableStageShrink;
	m_allowBossSpecificSpawns = l_requirements.allowBossSpecificSpawns;
	for (int i = 0; i < 4; ++i)
		m_bossArenaInset[i] = 0.0f;

	if (!m_bossArenaEnabled)
		return;

	m_bossArenaInset[0] = l_requirements.bossArenaBounds.topInsetTiles * l_blockSize;
	m_bossArenaInset[1] = l_requirements.bossArenaBounds.rightInsetTiles * l_blockSize;
	m_bossArenaInset[2] = l_requirements.bossArenaBounds.bottomInsetTiles * l_blockSize;
	m_bossArenaInset[3] = l_requirements.bossArenaBounds.leftInsetTiles * l_blockSize;
}

void World::ClearBossArenaMode()
{
	m_bossArenaEnabled = false;
	m_disableShrinkForBossArena = false;
	m_allowBossSpecificSpawns = false;
	for (int i = 0; i < 4; ++i)
		m_bossArenaInset[i] = 0.0f;
}

void World::ClampSnakeToPlayableGrid(Snake& l_snake)
{
	float bs = l_snake.GetBlockSize();
	float xLeft = GetEffectiveThickness(3) / bs;
	float xRight = m_maxX - GetEffectiveThickness(1) / bs - 1.0f;
	float yTop = (GetEffectiveThickness(0) + m_topOffset) / bs;
	float yBottom = m_maxY - GetEffectiveThickness(2) / bs - 1.0f;

	const int xMin = (int)std::ceil(xLeft - 1e-4f);
	const int xMax = (int)std::floor(xRight + 1e-4f);
	const int yMin = (int)std::ceil(yTop - 1e-4f);
	const int yMax = (int)std::floor(yBottom + 1e-4f);

	if (xMin > xMax || yMin > yMax)
		return;

	l_snake.ClampBodyToInclusiveGridBounds(xMin, xMax, yMin, yMax);
}

void World::SetBorderOffset(int l_side, float l_offsetPixels)
{
	if (l_side >= 0 && l_side < 4)
		m_borderOffset[l_side] = l_offsetPixels;
}

float World::GetBorderOffset(int l_side) const
{
	return (l_side >= 0 && l_side < 4) ? m_borderOffset[l_side] : 0.0f;
}

void World::ResetBorderOffsets()
{
	for (int i = 0; i < 4; i++)
		m_borderOffset[i] = 0.0f;
}

float World::GetEffectiveThickness(int l_side) const
{
	if (l_side >= 0 && l_side < 4)
		return m_borderThickness + m_borderOffset[l_side] + m_bossArenaInset[l_side];
	return m_borderThickness;
}

bool World::IsAppleInBounds(float l_blockSize) const
{
	float bs = l_blockSize;
	float xLeft = GetEffectiveThickness(3) / bs;
	float xRight = m_maxX - GetEffectiveThickness(1) / bs - 2 * (m_appleRaduis / bs);
	float yTop = (GetEffectiveThickness(0) + m_topOffset) / bs;
	float yBottom = m_maxY - GetEffectiveThickness(2) / bs - 2 * (m_appleRaduis / bs);

	return m_applePos.x >= xLeft && m_applePos.x <= xRight &&
		   m_applePos.y >= yTop && m_applePos.y <= yBottom;
}

void World::SetAppleColor(sf::Color l_color)
{
	m_apple.setFillColor(l_color);
	m_appleColor = l_color;
}

void World::Render(Window& l_window)
{
	if (m_useInkStyle)
	{
		RenderInk(l_window.GetRenderWindow(), 0.0f);
		return;
	}

	for (int i = 0; i < 4; i++)
		l_window.Draw(m_borders[i]);

	l_window.Draw(m_apple);
}

void World::RenderInk(sf::RenderTarget& l_target, float l_gameTime)
{
	RenderInkBorders(l_target);
	RenderInkApple(l_target, l_gameTime);
}

void World::RenderInkBorders(sf::RenderTarget& l_target)
{
	// Draw borders with cross-hatch fill and torn edges
	sf::Color borderFill = m_normalBorderColor;
	if (m_flashTimer > 0.0f)
		borderFill = sf::Color::Red;

	for (int i = 0; i < 4; i++)
	{
		sf::Vector2f pos = m_borders[i].getPosition();
		sf::Vector2f size = m_borders[i].getSize();

		// Fill with cross-hatching
		sf::Color hatchColor(m_inkTint.r, m_inkTint.g, m_inkTint.b,
							 (sf::Uint8)(borderFill.a > 0 ? 100 : 0));

		// Draw base fill
		sf::RectangleShape base(size);
		base.setPosition(pos);
		base.setFillColor(borderFill);
		l_target.draw(base);

		// Overlay cross-hatch lines
		InkRenderer::DrawCrossHatch(l_target, pos.x, pos.y, size.x, size.y,
									hatchColor, 4.0f, 45.0f, m_corruption,
									(unsigned int)(i * 997));

		// Draw torn edge on inner side
		switch (i)
		{
			case 0: // Top border - torn at bottom edge
				InkRenderer::DrawTornEdge(l_target, pos.x, pos.y + size.y, size.x,
										  true, true, borderFill, m_corruption, 100 + i);
				break;
			case 1: // Right border - torn at left edge
				InkRenderer::DrawTornEdge(l_target, pos.x, pos.y, size.y,
										  false, true, borderFill, m_corruption, 100 + i);
				break;
			case 2: // Bottom border - torn at top edge
				InkRenderer::DrawTornEdge(l_target, pos.x, pos.y, size.x,
										  true, false, borderFill, m_corruption, 100 + i);
				break;
			case 3: // Left border - torn at right edge
				InkRenderer::DrawTornEdge(l_target, pos.x + size.x, pos.y, size.y,
										  false, false, borderFill, m_corruption, 100 + i);
				break;
			default:
				break;
		}
	}

	// Draw margin line (red vertical line at inner edge of left border)
	float leftInner = m_borders[3].getPosition().x + m_borders[3].getSize().x;
	sf::RectangleShape marginLine(sf::Vector2f(2.0f,
		(float)m_borders[3].getSize().y));
	marginLine.setPosition(leftInner, m_borders[3].getPosition().y);
	marginLine.setFillColor(sf::Color(200, 60, 60, 120));
	l_target.draw(marginLine);
}

void World::RenderInkApple(sf::RenderTarget& l_target, float l_gameTime)
{
	float bs = m_appleRaduis * 2.0f;
	float cx = m_applePos.x * bs + m_appleRaduis;
	float cy = m_applePos.y * bs + m_appleRaduis;

	// Breathing animation
	float breathScale = 1.0f + std::sin(l_gameTime * 3.14f) * 0.05f;
	float radius = m_appleRaduis * breathScale;

	// Wobbly circle
	InkRenderer::DrawWobblyCircle(l_target, cx, cy, radius,
								  m_appleColor,
								  sf::Color(m_inkTint.r, m_inkTint.g, m_inkTint.b, 200),
								  1.5f, m_corruption, m_appleSeed, 16);

	// Small leaf (V shape at top)
	sf::Color leafColor(m_inkTint.r, m_inkTint.g, m_inkTint.b, 180);
	InkRenderer::DrawWobblyLine(l_target,
								cx, cy - radius,
								cx + 3.0f, cy - radius - 4.0f,
								leafColor, 1.0f, m_corruption * 0.5f, m_appleSeed + 1);
	InkRenderer::DrawWobblyLine(l_target,
								cx, cy - radius,
								cx - 2.0f, cy - radius - 3.0f,
								leafColor, 1.0f, m_corruption * 0.5f, m_appleSeed + 2);

	// Tiny highlight dot
	sf::CircleShape highlight(1.5f);
	highlight.setOrigin(1.5f, 1.5f);
	highlight.setPosition(cx - radius * 0.3f, cy - radius * 0.3f);
	highlight.setFillColor(sf::Color(255, 255, 255, 80));
	l_target.draw(highlight);
}