#pragma once

#include "Platform/Platform.hpp"
#include "Boss.h"
#include "Snake.h"
#include "Textbox.h"
#include "Window.h"

class World
{
public:
	World(Window& l_window, Snake& l_snake);
	~World();

	void Reset(Window& l_window, Snake& l_snake);
	void Update(Window& l_window, Snake& l_snake);
	void Render(Window& l_window);
	void RenderInk(sf::RenderTarget& l_target, float l_gameTime); // Ink-style rendering
	void Borders(Window& l_window);
	void RespawnApple(Snake& l_snake);
	void NarrowWorld(Window& l_window, Snake& l_snake);
	void CheckCollision(Window& l_window, Snake& l_snake);

	inline sf::Vector2f GetApplePos() const
	{
		return m_applePos;
	}

	inline int GetApplesEaten() const
	{
		return m_totalApplesEaten;
	}

	inline int GetShrinkCount() const
	{
		return m_shrinkCount;
	}

	void SetBorderColor(sf::Color l_color);
	sf::Color GetBorderColor() const { return m_normalBorderColor; }
	void SetTopOffset(float l_offset);
	void FlashBorders(float l_duration);
	void UpdateFlash(float l_dt);

	void SetShrinkInterval(int l_interval);
	void SetShrinkTimerSec(float l_sec);
	void UpdateTimedShrink(float l_dt, Window& l_window, Snake& l_snake);
	void TriggerShrink(Window& l_window, Snake& l_snake);
	void SetAppleColor(sf::Color l_color);
	void SetBossArenaMode(const BossArenaRequirements& l_requirements, float l_blockSize);
	void ClearBossArenaMode();
	bool IsBossArenaMode() const { return m_bossArenaEnabled; }

	// After borders/arena change, keep the snake inside the playable grid (matches CheckCollision).
	void ClampSnakeToPlayableGrid(Snake& l_snake);

	// Per-side border offsets (for earthquake mechanic)
	void SetBorderOffset(int l_side, float l_offsetPixels);
	float GetBorderOffset(int l_side) const;
	void ResetBorderOffsets();
	float GetEffectiveThickness(int l_side) const;
	bool IsAppleInBounds(float l_blockSize) const;

	inline float GetFlashTimer() const { return m_flashTimer; }
	inline float GetBorderThickness() const { return m_borderThickness; }
	inline float GetMaxX() const { return m_maxX; }
	inline float GetMaxY() const { return m_maxY; }
	inline float GetTopOffset() const { return m_topOffset; }

	// Level context (for phantom rules)
	void SetLevelId(int l_id) { m_levelId = l_id; }

	// Ink style
	void SetUseInkStyle(bool l_use) { m_useInkStyle = l_use; }
	void SetCorruption(float l_corruption) { m_corruption = l_corruption; }
	void SetInkTint(const sf::Color& l_tint) { m_inkTint = l_tint; }
	void SetAccentColor(const sf::Color& l_accent) { m_accentColor = l_accent; }

private:
	void RenderInkBorders(sf::RenderTarget& l_target);
	void RenderInkApple(sf::RenderTarget& l_target, float l_gameTime);

	sf::RectangleShape m_borders[4];
	sf::CircleShape m_apple;

	sf::Vector2f m_applePos;

	int m_count;
	int m_totalApplesEaten;
	int m_shrinkCount;

	float m_maxX;
	float m_maxY;
	float m_appleRaduis;
	float m_borderThickness;
	float m_topOffset;

	float m_flashTimer;
	sf::Color m_normalBorderColor;

	int m_shrinkInterval;
	float m_shrinkTimerSec;
	float m_shrinkTimerAccum;

	float m_borderOffset[4]; // per-side offset in pixels: 0=top, 1=right, 2=bottom, 3=left
	float m_bossArenaInset[4];
	int m_levelId;
	bool m_bossArenaEnabled;
	bool m_disableShrinkForBossArena;
	bool m_allowBossSpecificSpawns; // set from BossArenaRequirements; spawn logic TBD

	// Ink style
	bool m_useInkStyle;
	float m_corruption;
	sf::Color m_inkTint;
	sf::Color m_accentColor;
	sf::Color m_appleColor;
	unsigned int m_appleSeed; // Random seed for apple wobble shape
};

inline float Random(int a, int b)
{
	return (a + rand() % (b - a + 1));
}
