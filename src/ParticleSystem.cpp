#include "ParticleSystem.h"
#include "Snake.h"
#include "LevelConfig.h"
#include "RandomUtils.h"
#include <cmath>
#include <algorithm>

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
		// Apply gravity for InkDrip
		if (p.type == ParticleType::InkDrip)
			p.velocity.y += p.gravity * l_dt;
		// Rotate
		p.rotation += l_dt * 30.0f; // gentle spin
	}

	// Remove dead particles
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const Particle& p) { return p.age >= p.lifetime; }),
		m_particles.end());
}

void ParticleSystem::Render(Window& l_window)
{
	RenderTo(l_window.GetRenderWindow());
}

void ParticleSystem::RenderTo(sf::RenderTarget& l_target)
{
	for (auto& p : m_particles)
	{
		float alpha = 255.0f * (1.0f - p.age / p.lifetime);
		if (alpha < 0.0f) alpha = 0.0f;
		sf::Uint8 a = (sf::Uint8)alpha;

		switch (p.type)
		{
			case ParticleType::Square:
			{
				sf::RectangleShape rect(sf::Vector2f(p.size, p.size));
				rect.setPosition(p.position);
				rect.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
				l_target.draw(rect);
				break;
			}
			case ParticleType::Text:
			{
				if (!m_fontLoaded) break;
				sf::Text text;
				text.setFont(m_font);
				text.setString(p.text);
				text.setCharacterSize((unsigned int)p.size);
				text.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
				text.setPosition(p.position);
				l_target.draw(text);
				break;
			}
			case ParticleType::InkSplat:
			{
				sf::CircleShape blob(p.size, (size_t)p.pointCount);
				blob.setOrigin(p.size, p.size);
				blob.setPosition(p.position);
				blob.setRotation(p.rotation);
				blob.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
				l_target.draw(blob);
				break;
			}
			case ParticleType::InkDrip:
			{
				float stretch = 1.0f + p.age * 2.0f;
				sf::RectangleShape drip(sf::Vector2f(p.size * 0.6f, p.size * stretch));
				drip.setOrigin(p.size * 0.3f, 0);
				drip.setPosition(p.position);
				drip.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
				l_target.draw(drip);
				break;
			}
			case ParticleType::InkDot:
			{
				sf::CircleShape dot(p.size);
				dot.setOrigin(p.size, p.size);
				dot.setPosition(p.position);
				dot.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, a));
				l_target.draw(dot);
				break;
			}
			default:
				break;
		}
	}
}

void ParticleSystem::Clear()
{
	m_particles.clear();
}

void ParticleSystem::SpawnAppleBurst(const sf::Vector2f& l_pos, const sf::Color& l_color)
{
	int count = RandomInt(8, 12);

	for (int i = 0; i < count; i++)
	{
		Particle p{};
		p.type = ParticleType::InkSplat;
		p.position = l_pos;

		float angle = RandomFloat(0.0f, 2.0f * (float)M_PI);
		float speed = RandomFloat(100.0f, 200.0f);
		p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

		p.lifetime = 0.3f;
		p.age = 0.0f;
		p.color = l_color;
		p.size = RandomFloat(2.0f, 4.0f);
		p.gravity = 0.0f;
		p.rotation = RandomFloat(0.0f, 360.0f);
		p.pointCount = RandomInt(5, 8);
		p.seed = (unsigned int)i;

		m_particles.push_back(p);
	}
}

void ParticleSystem::SpawnSelfCollisionCut(const std::vector<Position>& l_segments, float l_blockSize,
										   const sf::Color& l_color)
{
	for (const auto& seg : l_segments)
	{
		Particle p{};
		p.type = ParticleType::InkDrip;
		p.position = sf::Vector2f(seg.x * l_blockSize, seg.y * l_blockSize);

		// Drift downward with slight random horizontal
		float hSpeed = RandomFloat(-20.0f, 20.0f);
		p.velocity = sf::Vector2f(hSpeed, RandomFloat(30.0f, 60.0f));

		p.lifetime = 0.6f;
		p.age = 0.0f;
		p.color = l_color;
		p.size = l_blockSize * 0.5f;
		p.gravity = 200.0f;
		p.rotation = 0.0f;
		p.pointCount = 0;
		p.seed = 0;

		m_particles.push_back(p);
	}
}

void ParticleSystem::SpawnFloatingText(const std::string& l_text, const sf::Vector2f& l_pos,
									   const sf::Color& l_color)
{
	if (!m_fontLoaded)
		return;

	Particle p{};
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

void ParticleSystem::SpawnInkSplat(const sf::Vector2f& l_pos, const sf::Color& l_color, int l_count)
{
	for (int i = 0; i < l_count; i++)
	{
		Particle p{};
		p.type = ParticleType::InkSplat;
		p.position = l_pos + sf::Vector2f(RandomFloat(-30.0f, 30.0f), RandomFloat(-30.0f, 30.0f));
		p.velocity = sf::Vector2f(RandomFloat(-10.0f, 10.0f), RandomFloat(-10.0f, 10.0f));
		p.lifetime = 1.0f;
		p.age = 0.0f;
		p.color = l_color;
		p.size = RandomFloat(2.0f, 5.0f);
		p.gravity = 0.0f;
		p.rotation = RandomFloat(0.0f, 360.0f);
		p.pointCount = RandomInt(5, 8);
		p.seed = (unsigned int)i;
		m_particles.push_back(p);
	}
}

void ParticleSystem::SpawnInkDrips(const sf::Vector2f& l_pos, const sf::Color& l_color, int l_count)
{
	for (int i = 0; i < l_count; i++)
	{
		Particle p{};
		p.type = ParticleType::InkDrip;
		p.position = l_pos + sf::Vector2f(RandomFloat(-5.0f, 5.0f), 0.0f);
		p.velocity = sf::Vector2f(RandomFloat(-5.0f, 5.0f), RandomFloat(20.0f, 50.0f));
		p.lifetime = 0.8f;
		p.age = 0.0f;
		p.color = l_color;
		p.size = RandomFloat(2.0f, 4.0f);
		p.gravity = 200.0f;
		p.rotation = 0.0f;
		p.pointCount = 0;
		p.seed = 0;
		m_particles.push_back(p);
	}
}

void ParticleSystem::SpawnInkDust(const sf::Vector2f& l_pos, const sf::Color& l_color, int l_count)
{
	for (int i = 0; i < l_count; i++)
	{
		Particle p{};
		p.type = ParticleType::InkDot;
		p.position = l_pos + sf::Vector2f(RandomFloat(-40.0f, 40.0f), RandomFloat(-40.0f, 40.0f));
		p.velocity = sf::Vector2f(RandomFloat(-8.0f, 8.0f), RandomFloat(-15.0f, -5.0f));
		p.lifetime = 1.5f;
		p.age = 0.0f;
		p.color = sf::Color(l_color.r, l_color.g, l_color.b, 120);
		p.size = RandomFloat(1.0f, 2.0f);
		p.gravity = 0.0f;
		p.rotation = 0.0f;
		p.pointCount = 0;
		p.seed = 0;
		m_particles.push_back(p);
	}
}
