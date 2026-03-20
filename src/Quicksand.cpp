#include "Quicksand.h"
#include "RandomUtils.h"
#include "InkRenderer.h"

Quicksand::Quicksand()
	: m_relocateTimer(0.0f),
	  m_relocateInterval(15.0f),
	  m_patchCount(3)
{
}

void Quicksand::Reset(float l_maxX, float l_maxY, float l_borderThickness, float l_blockSize, float l_topOffset)
{
	m_relocateTimer = 0.0f;
	GeneratePatches(l_maxX, l_maxY, l_borderThickness, l_blockSize, l_topOffset);
}

void Quicksand::Update(float l_dt, float l_maxX, float l_maxY, float l_borderThickness, float l_blockSize, float l_topOffset)
{
	m_relocateTimer += l_dt;
	if (m_relocateTimer >= m_relocateInterval)
	{
		GeneratePatches(l_maxX, l_maxY, l_borderThickness, l_blockSize, l_topOffset);
		m_relocateTimer = 0.0f;
	}
}

void Quicksand::GeneratePatches(float l_maxX, float l_maxY, float l_borderThickness, float l_blockSize, float l_topOffset)
{
	m_patches.clear();

	int minX = (int)(l_borderThickness / l_blockSize) + 1;
	int maxX = (int)(l_maxX - l_borderThickness / l_blockSize) - 4; // -3 for patch size, -1 for margin
	int minY = (int)((l_borderThickness + l_topOffset) / l_blockSize) + 1;
	int maxY = (int)(l_maxY - l_borderThickness / l_blockSize) - 4;

	if (maxX <= minX || maxY <= minY) return;

	for (int p = 0; p < m_patchCount; p++)
	{
		bool valid = false;
		int attempts = 0;
		QuicksandPatch patch;

		while (!valid && attempts < 50)
		{
			patch.x = RandomInt(minX, maxX);
			patch.y = RandomInt(minY, maxY);
			valid = true;

			// Check no overlap with existing patches
			for (const auto& existing : m_patches)
			{
				if (patch.x < existing.x + 4 && patch.x + 3 > existing.x - 1 &&
					patch.y < existing.y + 4 && patch.y + 3 > existing.y - 1)
				{
					valid = false;
					break;
				}
			}
			attempts++;
		}

		if (valid)
			m_patches.push_back(patch);
	}
}

void Quicksand::Render(Window& l_window, float l_blockSize)
{
	sf::RenderTarget& target = l_window.GetRenderWindow();

	for (const auto& patch : m_patches)
	{
		// Draw stipple dots for each cell in the 3x3 patch
		for (int dy = 0; dy < 3; dy++)
		{
			for (int dx = 0; dx < 3; dx++)
			{
				float px = (patch.x + dx) * l_blockSize;
				float py = (patch.y + dy) * l_blockSize;

				// Dense stipple dots (ink-on-paper look)
				int dotCount = 12 + ((patch.x + dx + patch.y + dy) % 5);
				unsigned int seed = (unsigned int)(patch.x * 31 + patch.y * 97 + dx * 7 + dy * 13);

				for (int d = 0; d < dotCount; d++)
				{
					unsigned int h = InkRenderer::Hash(seed, (unsigned int)d);
					float ox = (float)(h % (int)l_blockSize);
					float oy = (float)((h >> 8) % (int)l_blockSize);
					int alpha = 60 + (int)((h >> 16) % 60);

					sf::CircleShape dot(1.0f);
					dot.setPosition(px + ox, py + oy);
					dot.setFillColor(sf::Color(120, 80, 30, (sf::Uint8)alpha));
					target.draw(dot);
				}
			}
		}

		// Draw a subtle wobbly outline around the 3x3 patch
		float patchPx = patch.x * l_blockSize;
		float patchPy = patch.y * l_blockSize;
		float patchSize = 3.0f * l_blockSize;
		InkRenderer::DrawWobblyRect(target,
									patchPx, patchPy, patchSize, patchSize,
									sf::Color::Transparent,
									sf::Color(120, 80, 30, 40),
									0.5f, 0.2f,
									(unsigned int)(patch.x * 100 + patch.y));
	}
}

bool Quicksand::IsOnQuicksand(const Position& l_pos) const
{
	for (const auto& patch : m_patches)
	{
		if (l_pos.x >= patch.x && l_pos.x < patch.x + 3 &&
			l_pos.y >= patch.y && l_pos.y < patch.y + 3)
			return true;
	}
	return false;
}
