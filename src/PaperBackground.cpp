#include "PaperBackground.h"
#include "RandomUtils.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PaperBackground::PaperBackground()
	: m_doodleFontLoaded(false), m_generated(false)
{
	m_doodleFontLoaded = m_doodleFont.loadFromFile(FONT_PATH);
}

void PaperBackground::Generate(const LevelConfig& l_config, unsigned int l_windowWidth,
							   unsigned int l_windowHeight)
{
	m_generated = false;

	if (!m_renderTexture.create(l_windowWidth, l_windowHeight))
		return;

	m_renderTexture.clear(l_config.paperTone);

	// Draw noise overlay for paper fiber texture
	m_noiseTexture = TextureGenerator::GenerateNoiseTexture(256, 256, (unsigned int)l_config.id * 7919);
	sf::Sprite noiseSpr(m_noiseTexture);
	noiseSpr.setColor(sf::Color(
		l_config.inkTint.r, l_config.inkTint.g, l_config.inkTint.b, 15));
	// Tile the noise across the background
	for (unsigned int y = 0; y < l_windowHeight; y += 256)
	{
		for (unsigned int x = 0; x < l_windowWidth; x += 256)
		{
			noiseSpr.setPosition((float)x, (float)y);
			m_renderTexture.draw(noiseSpr);
		}
	}

	DrawRuledLines(m_renderTexture, l_config, l_windowWidth, l_windowHeight);
	DrawMarginLine(m_renderTexture, l_config, l_windowHeight);
	DrawAgeStains(m_renderTexture, l_config, l_windowWidth, l_windowHeight);
	DrawMarginDoodle(m_renderTexture, l_config, l_windowWidth, l_windowHeight);

	m_renderTexture.display();
	m_sprite.setTexture(m_renderTexture.getTexture(), true);
	m_sprite.setPosition(0, 0);
	m_generated = true;
}

void PaperBackground::Render(sf::RenderTarget& l_target)
{
	if (m_generated)
		l_target.draw(m_sprite);
}

void PaperBackground::DrawRuledLines(sf::RenderTarget& l_target, const LevelConfig& l_config,
									 unsigned int l_width, unsigned int l_height)
{
	sf::Color lineColor(l_config.inkTint.r, l_config.inkTint.g, l_config.inkTint.b,
						(sf::Uint8)(15 + l_config.corruption * 10));

	float spacing = 32.0f; // Every 2 grid blocks
	unsigned int seed = (unsigned int)(l_config.id * 31);

	for (float y = spacing; y < (float)l_height; y += spacing)
	{
		float wobbleAmp = l_config.corruption * 2.0f;
		if (wobbleAmp < 0.1f)
		{
			// Nearly straight line
			sf::RectangleShape line(sf::Vector2f((float)l_width, 1.0f));
			line.setPosition(0.0f, y);
			line.setFillColor(lineColor);
			l_target.draw(line);
		}
		else
		{
			InkRenderer::DrawWobblyLine(l_target, 0, y, (float)l_width, y,
										lineColor, 1.0f, l_config.corruption * 0.3f,
										seed + (unsigned int)(y), 16);
		}
	}
}

void PaperBackground::DrawMarginLine(sf::RenderTarget& l_target, const LevelConfig& /*l_config*/,
									 unsigned int l_height)
{
	sf::Color marginColor(200, 60, 60, 40);
	sf::RectangleShape margin(sf::Vector2f(2.0f, (float)l_height));
	margin.setPosition(48.0f, 0.0f);
	margin.setFillColor(marginColor);
	l_target.draw(margin);
}

void PaperBackground::DrawAgeStains(sf::RenderTarget& l_target, const LevelConfig& l_config,
									unsigned int l_width, unsigned int l_height)
{
	int stainCount = 3 + (int)(l_config.corruption * 3);
	unsigned int seed = (unsigned int)(l_config.id * 4999);

	for (int i = 0; i < stainCount; i++)
	{
		unsigned int h = TextureGenerator::Hash(seed + (unsigned int)i);
		float x = (float)(h % l_width);
		float y = (float)((h >> 8) % l_height);
		float radius = 80.0f + (float)((h >> 16) % 120);
		sf::Uint8 alpha = (sf::Uint8)(25 + (h >> 24) % 25);

		sf::CircleShape stain(radius);
		stain.setOrigin(radius, radius);
		stain.setPosition(x, y);
		stain.setFillColor(sf::Color(180, 160, 120, alpha));
		l_target.draw(stain);
	}
}

void PaperBackground::DrawMarginDoodle(sf::RenderTarget& l_target, const LevelConfig& l_config,
									   unsigned int /*l_width*/, unsigned int l_height)
{
	sf::Color doodleColor(l_config.inkTint.r, l_config.inkTint.g, l_config.inkTint.b, 60);
	float bx = 15.0f; // In the left margin
	float by = (float)l_height - 80.0f; // Bottom area

	switch (l_config.id)
	{
		case 1: // Smiley face
		{
			InkRenderer::DrawWobblyCircle(l_target, bx + 12, by + 12, 10,
										  sf::Color::Transparent, doodleColor, 1.0f, 0.1f, 101);
			// Eyes
			sf::CircleShape eye(1.5f);
			eye.setFillColor(doodleColor);
			eye.setPosition(bx + 7, by + 8);
			l_target.draw(eye);
			eye.setPosition(bx + 14, by + 8);
			l_target.draw(eye);
			// Smile arc (simplified as a line)
			InkRenderer::DrawWobblyLine(l_target, bx + 7, by + 16, bx + 17, by + 16,
										doodleColor, 1.0f, 0.15f, 102);
			break;
		}
		case 2: // Eye, crossed out
		{
			InkRenderer::DrawWobblyCircle(l_target, bx + 12, by + 12, 8,
										  sf::Color::Transparent, doodleColor, 1.0f, 0.15f, 201);
			sf::CircleShape pupil(3.0f);
			pupil.setFillColor(doodleColor);
			pupil.setPosition(bx + 9, by + 9);
			l_target.draw(pupil);
			// X through it
			InkRenderer::DrawWobblyLine(l_target, bx + 2, by + 2, bx + 22, by + 22,
										doodleColor, 1.5f, 0.2f, 202);
			InkRenderer::DrawWobblyLine(l_target, bx + 22, by + 2, bx + 2, by + 22,
										doodleColor, 1.5f, 0.2f, 203);
			break;
		}
		case 3: // Spiral
		{
			for (int i = 0; i < 30; i++)
			{
				float a1 = (float)i * 0.6f;
				float a2 = a1 + 0.6f;
				float r1 = 3.0f + a1 * 1.2f;
				float r2 = 3.0f + a2 * 1.2f;
				sf::Vertex line[] = {
					sf::Vertex(sf::Vector2f(bx + 12 + std::cos(a1) * r1, by + 12 + std::sin(a1) * r1), doodleColor),
					sf::Vertex(sf::Vector2f(bx + 12 + std::cos(a2) * r2, by + 12 + std::sin(a2) * r2), doodleColor)
				};
				l_target.draw(line, 2, sf::Lines);
			}
			break;
		}
		case 4: // Two mirrored stick figures
		{
			// Left figure
			InkRenderer::DrawWobblyCircle(l_target, bx + 6, by + 5, 4,
										  sf::Color::Transparent, doodleColor, 1.0f, 0.15f, 401);
			InkRenderer::DrawWobblyLine(l_target, bx + 6, by + 9, bx + 6, by + 20,
										doodleColor, 1.0f, 0.1f, 402);
			// Right figure (mirrored)
			InkRenderer::DrawWobblyCircle(l_target, bx + 20, by + 5, 4,
										  sf::Color::Transparent, doodleColor, 1.0f, 0.15f, 403);
			InkRenderer::DrawWobblyLine(l_target, bx + 20, by + 9, bx + 20, by + 20,
										doodleColor, 1.0f, 0.1f, 404);
			break;
		}
		case 5: // Empty plate
		{
			InkRenderer::DrawWobblyCircle(l_target, bx + 12, by + 12, 10,
										  sf::Color::Transparent, doodleColor, 1.0f, 0.2f, 501);
			InkRenderer::DrawWobblyCircle(l_target, bx + 12, by + 12, 6,
										  sf::Color::Transparent, doodleColor, 0.5f, 0.15f, 502);
			break;
		}
		case 6: // Apple with X
		{
			InkRenderer::DrawWobblyCircle(l_target, bx + 12, by + 14, 8,
										  sf::Color::Transparent, doodleColor, 1.0f, 0.2f, 601);
			// Leaf
			InkRenderer::DrawWobblyLine(l_target, bx + 12, by + 6, bx + 16, by + 3,
										doodleColor, 1.0f, 0.1f, 602);
			// X
			InkRenderer::DrawWobblyLine(l_target, bx + 5, by + 8, bx + 19, by + 20,
										sf::Color(200, 60, 60, 80), 1.5f, 0.2f, 603);
			InkRenderer::DrawWobblyLine(l_target, bx + 19, by + 8, bx + 5, by + 20,
										sf::Color(200, 60, 60, 80), 1.5f, 0.2f, 604);
			break;
		}
		case 7: // Zigzag earthquake
		{
			float px = bx + 2;
			float py = by + 5;
			for (int i = 0; i < 6; i++)
			{
				float nx = px + 4;
				float ny = py + ((i % 2 == 0) ? 8.0f : -8.0f);
				InkRenderer::DrawWobblyLine(l_target, px, py, nx, ny,
											doodleColor, 1.0f, 0.25f, 700 + i);
				px = nx;
				py = ny;
			}
			break;
		}
		case 8: // Multiple eyes
		{
			for (int i = 0; i < 5; i++)
			{
				float ex = bx + 3 + (i % 3) * 8.0f;
				float ey = by + 3 + (i / 3) * 12.0f;
				InkRenderer::DrawWobblyCircle(l_target, ex, ey, 4.0f,
											  sf::Color::Transparent, doodleColor, 0.8f,
											  0.15f, 800 + i);
				sf::CircleShape pupil(1.5f);
				pupil.setFillColor(doodleColor);
				pupil.setPosition(ex - 1.5f, ey - 1.5f);
				l_target.draw(pupil);
			}
			break;
		}
		case 9: // Question marks
		{
			if (m_doodleFontLoaded)
			{
				for (int i = 0; i < 4; i++)
				{
					sf::Text q;
					q.setFont(m_doodleFont);
					q.setString("?");
					q.setCharacterSize(12 + i * 2);
					q.setFillColor(doodleColor);
					q.setPosition(bx + (i % 2) * 10.0f, by + (i / 2) * 12.0f);
					if (i % 2 == 1) q.setRotation(180.0f);
					l_target.draw(q);
				}
			}
			break;
		}
		case 10: // All doodles overlapping chaotically
		{
			// Draw small versions of several doodles overlapping
			sf::Color chaosColor(l_config.inkTint.r, l_config.inkTint.g, l_config.inkTint.b, 40);
			// Smiley
			InkRenderer::DrawWobblyCircle(l_target, bx + 8, by + 8, 6,
										  sf::Color::Transparent, chaosColor, 0.8f, 0.4f, 1001);
			// Eye
			InkRenderer::DrawWobblyCircle(l_target, bx + 18, by + 5, 5,
										  sf::Color::Transparent, chaosColor, 0.8f, 0.4f, 1002);
			// Zigzag
			InkRenderer::DrawWobblyLine(l_target, bx + 2, by + 18, bx + 24, by + 22,
										chaosColor, 1.0f, 0.5f, 1003);
			// Question marks
			InkRenderer::DrawWobblyLine(l_target, bx + 5, by + 15, bx + 20, by + 10,
										chaosColor, 1.0f, 0.6f, 1004);
			// X
			InkRenderer::DrawWobblyLine(l_target, bx, by, bx + 26, by + 26,
										sf::Color(200, 60, 60, 50), 1.0f, 0.5f, 1005);
			break;
		}
		default:
			break;
	}
}
