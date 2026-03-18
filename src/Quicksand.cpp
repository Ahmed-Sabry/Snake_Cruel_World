#include "Quicksand.h"
#include "RandomUtils.h"

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
	m_patchRect.setSize(sf::Vector2f(l_blockSize - 1, l_blockSize - 1));

	for (const auto& patch : m_patches)
	{
		for (int dy = 0; dy < 3; dy++)
		{
			for (int dx = 0; dx < 3; dx++)
			{
				// Slight alpha variation for visual texture
				int alphaVar = ((patch.x + dx + patch.y + dy) % 3) * 15;
				m_patchRect.setFillColor(sf::Color(140, 100, 40, (sf::Uint8)(105 + alphaVar)));
				m_patchRect.setPosition((patch.x + dx) * l_blockSize, (patch.y + dy) * l_blockSize);
				l_window.Draw(m_patchRect);
			}
		}
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
