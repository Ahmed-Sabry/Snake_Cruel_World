#include "ParticleSystem.h"
#include "Snake.h"
#include "LevelConfig.h"
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ParticleSystem::ParticleSystem()
	: m_fontLoaded(false)
{
	m_fontLoaded = m_font.loadFromFile(FONT_PATH);
}

void ParticleSystem::Update(float l_dt)
{
	for (auto& p : m_particles)
	{
		p.age += l_dt;
		p.position += p.velocity * l_dt;
	}

	// Remove dead particles
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const Particle& p) { return p.age >= p.lifetime; }),
		m_particles.end());
}

void ParticleSystem::Render(Window& l_window)
{
	for (auto& p : m_particles)
	{
		float alpha = 255.0f * (1.0f - p.age / p.lifetime);
		if (alpha < 0.0f) alpha = 0.0f;
		sf::Uint8 a = (sf::Uint8)alpha;

		if (p.type == ParticleType::Square)
		{
			sf::RectangleShape rect(sf::Vector2f(p.size, p.size));
			rect.setPosition(p.position);
			rect.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
			l_window.Draw(rect);
		}
		else if (p.type == ParticleType::Text && m_fontLoaded)
		{
			sf::Text text;
			text.setFont(m_font);
			text.setString(p.text);
			text.setCharacterSize((unsigned int)p.size);
			text.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
			text.setPosition(p.position);
			l_window.Draw(text);
		}
	}
}

void ParticleSystem::Clear()
{
	m_particles.clear();
}

void ParticleSystem::SpawnAppleBurst(const sf::Vector2f& l_pos, const sf::Color& l_color)
{
	int count = 8 + (rand() % 5); // 8-12 particles

	for (int i = 0; i < count; i++)
	{
		Particle p;
		p.type = ParticleType::Square;
		p.position = l_pos;

		float angle = (float)(rand() % 360) * (float)M_PI / 180.0f;
		float speed = 100.0f + (float)(rand() % 100); // 100-200 px/s
		p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

		p.lifetime = 0.3f;
		p.age = 0.0f;
		p.color = l_color;
		p.size = 3.0f + (float)(rand() % 3); // 3-5 px

		m_particles.push_back(p);
	}
}

void ParticleSystem::SpawnSelfCollisionCut(const std::vector<Position>& l_segments, float l_blockSize)
{
	for (const auto& seg : l_segments)
	{
		Particle p;
		p.type = ParticleType::Square;
		p.position = sf::Vector2f(seg.x * l_blockSize, seg.y * l_blockSize);

		// Drift downward with slight random horizontal
		float hSpeed = ((rand() % 100) / 100.0f - 0.5f) * 40.0f;
		p.velocity = sf::Vector2f(hSpeed, 50.0f + (float)(rand() % 30));

		p.lifetime = 0.4f;
		p.age = 0.0f;
		p.color = sf::Color::White;
		p.size = l_blockSize;

		m_particles.push_back(p);
	}
}

void ParticleSystem::SpawnFloatingText(const std::string& l_text, const sf::Vector2f& l_pos,
									   const sf::Color& l_color)
{
	Particle p;
	p.type = ParticleType::Text;
	p.position = l_pos;
	p.velocity = sf::Vector2f(0.0f, -80.0f); // float upward
	p.lifetime = 0.8f;
	p.age = 0.0f;
	p.color = l_color;
	p.size = 16.0f; // character size
	p.text = l_text;

	m_particles.push_back(p);
}
