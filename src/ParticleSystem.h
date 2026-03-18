#pragma once

#include "Window.h"
#include <vector>
#include <string>

struct Position; // forward declare from Snake.h

enum class ParticleType
{
	Square,
	Text
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

private:
	std::vector<Particle> m_particles;
	sf::Font m_font;
	bool m_fontLoaded;
};
