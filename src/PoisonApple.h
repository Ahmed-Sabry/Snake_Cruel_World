#pragma once

#include "Window.h"
#include "Snake.h"

class World;

class PoisonApple
{
public:
	PoisonApple();

	void Reset(float l_blockSize);
	void Update(float l_dt);
	void Render(Window& l_window, float l_blockSize);
	void RenderTo(sf::RenderTarget& l_target, float l_blockSize);

	void SpawnPoison(const Snake& l_snake, const World& l_world, float l_blockSize);
	bool CheckCollision(const Position& l_snakeHead) const;

	void OnPoisonEaten();
	void OnRealAppleEaten();

	bool IsControlInverted() const;
	float GetSpeedMultiplier() const;
	int GetGrowAmount() const;
	int GetRealApplesEaten() const;
	sf::Vector2f GetPixelPos(float l_blockSize) const;
	bool IsInBounds(const World& l_world, float l_blockSize) const;

private:
	sf::CircleShape m_shape;
	sf::Vector2f m_pos; // grid coords

	bool m_controlInverted;
	float m_invertTimer;
	float m_speedBoostTimer;
	float m_speedBoostAmount;
	int m_consecutivePoisons;
	int m_realApplesEaten;
	float m_pulseTimer;
	int m_extraGrowth;
};
