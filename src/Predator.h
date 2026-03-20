#pragma once

#include "Window.h"
#include "Snake.h"
#include <vector>

class World;

enum class PredatorMode
{
	HUNTING_APPLE,
	HUNTING_PLAYER
};

class Predator
{
public:
	Predator();

	void Reset(float l_blockSize, const Snake& l_snake, const World& l_world);
	void Update(float l_dt, const World& l_world, const Snake& l_snake);
	void Render(Window& l_window, float l_blockSize);
	void RenderTo(sf::RenderTarget& l_target, float l_blockSize);

	bool HitPlayer(const Position& l_playerHead) const;

	PredatorMode GetMode() const;
	int GetApplesEaten() const;
	bool JustAteApple();
	bool JustStartedHunting();
	void OnPlayerAteApple();
	const std::vector<Position>& GetBody() const;

private:
	Direction ChooseDirection(const Position& l_target, const World& l_world,
							 const Snake& l_snake) const;
	bool IsWall(const Position& l_pos, const World& l_world) const;
	bool IsOwnBody(const Position& l_pos) const;
	void Move();
	void Grow();
	Position FindSpawnPosition(const Snake& l_snake, const World& l_world) const;

	std::vector<Position> m_body; // [0] = head
	Direction m_direction;
	float m_blockSize;
	float m_speed;        // ticks/sec, starts at 8, +1 per apple eaten
	float m_elapsedTime;

	PredatorMode m_mode;
	int m_applesEaten;
	float m_huntTimer;    // 10s countdown for HUNTING_PLAYER
	bool m_justAteApple;
	bool m_justStartedHunting;

	sf::RectangleShape m_bodyRect;
	sf::CircleShape m_eyeShape;
	float m_modeTransitionTimer; // visual flash on mode switch
};
