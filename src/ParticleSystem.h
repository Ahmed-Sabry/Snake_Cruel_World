#pragma once

#include "Window.h"
#include <vector>
#include <string>

struct Position; // forward declare from Snake.h

enum class ParticleType
{
	Square,
	Text,
	InkSplat,  // Lumpy irregular blob
	InkDrip,   // Elongated shape that accelerates downward
	InkDot     // Tiny circle that appears and fades
};

struct Particle
{
	ParticleType type;
	sf::Vector2f position;
	sf::Vector2f velocity;
	float lifetime;
	float age;
	sf::Color color;
	float size;
	std::string text; // only used for ParticleType::Text
	float gravity;    // downward acceleration (for InkDrip)
	float rotation;   // visual rotation angle
	int pointCount;   // number of points for InkSplat shape
	unsigned int seed; // for deterministic wobble
};

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem() = default;

	void Update(float l_dt);
	void Render(Window& l_window);
	void Clear();

	void SpawnAppleBurst(const sf::Vector2f& l_pos, const sf::Color& l_color);
	void SpawnSelfCollisionCut(const std::vector<Position>& l_segments, float l_blockSize,
							   const sf::Color& l_color);
	void SpawnFloatingText(const std::string& l_text, const sf::Vector2f& l_pos,
						   const sf::Color& l_color);

	// Ink-themed particle spawners
	void SpawnInkSplat(const sf::Vector2f& l_pos, const sf::Color& l_color, int l_count = 10);
	void SpawnInkDrips(const sf::Vector2f& l_pos, const sf::Color& l_color, int l_count = 5);
	void SpawnInkDust(const sf::Vector2f& l_pos, const sf::Color& l_color, int l_count = 8);

private:
	std::vector<Particle> m_particles;
	sf::Font m_font;
	bool m_fontLoaded;
};
