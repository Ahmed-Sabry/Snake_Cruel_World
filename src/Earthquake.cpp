#include "Earthquake.h"
#include "World.h"
#include "RandomUtils.h"
#include "InkRenderer.h"
#include <cmath>
#include <algorithm>

Earthquake::Earthquake()
	: m_state(QuakeState::Idle),
	  m_timer(0.0f),
	  m_quakeInterval(7.0f),   // 7s idle + 1s warning = 8s between quakes (per spec)
	  m_warningDuration(1.0f),
	  m_blockSize(16.0f),
	  m_justQuaked(false),
	  m_crackAlpha(0.0f)
{
	for (int i = 0; i < 4; i++)
		m_pendingOffset[i] = 0.0f;
}

void Earthquake::Reset(float l_blockSize)
{
	m_blockSize = l_blockSize;
	m_state = QuakeState::Idle;
	m_timer = 0.0f;
	m_justQuaked = false;
	m_crackAlpha = 0.0f;
	m_cracks.clear();
	for (int i = 0; i < 4; i++)
		m_pendingOffset[i] = 0.0f;
}

void Earthquake::Update(float l_dt, World& l_world, Window& l_window)
{
	m_timer += l_dt;

	switch (m_state)
	{
		case QuakeState::Idle:
		{
			if (m_timer >= m_quakeInterval)
			{
				m_state = QuakeState::Warning;
				m_timer = 0.0f;

				float winW = (float)l_window.GetWindowSize().x;
				float winH = (float)l_window.GetWindowSize().y;
				GenerateQuake(l_world, winW, winH);
				GenerateCrackLines(l_world, l_window);
			}
			break;
		}

		case QuakeState::Warning:
		{
			// Pulse crack alpha during warning
			m_crackAlpha = 150.0f + 105.0f * std::sin(m_timer * 12.0f);

			if (m_timer >= m_warningDuration)
			{
				ApplyQuake(l_world, l_window);
				m_state = QuakeState::Quaking;
				m_timer = 0.0f;
				m_justQuaked = true;
				m_crackAlpha = 255.0f;
			}
			break;
		}

		case QuakeState::Quaking:
		{
			// Fade out crack lines over 0.5 seconds
			float fadeDuration = 0.5f;
			m_crackAlpha = 255.0f * (1.0f - m_timer / fadeDuration);
			if (m_crackAlpha < 0.0f) m_crackAlpha = 0.0f;

			if (m_timer >= fadeDuration)
			{
				m_state = QuakeState::Idle;
				m_timer = 0.0f;
				m_cracks.clear();
				m_crackAlpha = 0.0f;
			}
			break;
		}

		default:
			break;
	}
}

void Earthquake::GenerateQuake(const World& l_world, float l_windowWidth, float l_windowHeight)
{
	// Start from current offsets
	for (int i = 0; i < 4; i++)
		m_pendingOffset[i] = l_world.GetBorderOffset(i);

	float baseThick = l_world.GetBorderThickness();
	float topOffset = l_world.GetTopOffset();

	// Choose quake type: 60% shift, 40% reshape
	bool isReshape = (RandomInt(1, 10) <= 4);

	// Opposite sides: top(0)<->bottom(2), right(1)<->left(3)
	static const int opposite[] = { 2, 3, 0, 1 };

	for (int retry = 0; retry < 4; retry++)
	{
		float testOffset[4];
		for (int i = 0; i < 4; i++)
			testOffset[i] = m_pendingOffset[i];

		if (!isReshape)
		{
			// Uniform Shift: pick direction and amount

			int dir = RandomInt(0, 3);
			int amount = RandomInt(1, 2);
			float shift = amount * m_blockSize;

			testOffset[dir] += shift;
			testOffset[opposite[dir]] -= shift;
		}
		else
		{
			// Reshape: one axis closes, other expands

			int axis = RandomInt(0, 1); // 0=horizontal(left/right), 1=vertical(top/bottom)

			int closeAmt = RandomInt(1, 2);
			int expandAmt = RandomInt(1, 2);
			float closeShift = closeAmt * m_blockSize;
			float expandShift = expandAmt * m_blockSize;

			if (axis == 0)
			{
				// Left/right close, top/bottom expand
				testOffset[1] += closeShift;   // right closes
				testOffset[3] += closeShift;   // left closes
				testOffset[0] -= expandShift;  // top expands
				testOffset[2] -= expandShift;  // bottom expands
			}
			else
			{
				// Top/bottom close, left/right expand
				testOffset[0] += closeShift;   // top closes
				testOffset[2] += closeShift;   // bottom closes
				testOffset[1] -= expandShift;  // right expands
				testOffset[3] -= expandShift;  // left expands
			}
		}

		if (ValidateOffsets(testOffset, baseThick, l_windowWidth, l_windowHeight, topOffset, m_blockSize))
		{
			for (int i = 0; i < 4; i++)
				m_pendingOffset[i] = testOffset[i];
			return;
		}

		// Retry with opposite type on second attempt
		if (retry == 1) isReshape = !isReshape;
	}

	// All retries failed — skip this quake (keep current offsets)
}

void Earthquake::ApplyQuake(World& l_world, Window& l_window)
{
	for (int i = 0; i < 4; i++)
		l_world.SetBorderOffset(i, m_pendingOffset[i]);

	l_world.Borders(l_window);
}

bool Earthquake::ValidateOffsets(const float l_offsets[4], float l_baseThickness,
								 float l_windowW, float l_windowH, float l_topOffset, float l_blockSize) const
{
	// Each effective thickness must be >= 1 block
	for (int i = 0; i < 4; i++)
	{
		if (l_baseThickness + l_offsets[i] < l_blockSize)
			return false;
	}

	// Playable area must be >= 10 blocks in each dimension
	float effLeft   = l_baseThickness + l_offsets[3];
	float effRight  = l_baseThickness + l_offsets[1];
	float effTop    = l_baseThickness + l_offsets[0];
	float effBottom = l_baseThickness + l_offsets[2];

	float playableWidth  = l_windowW - effLeft - effRight;
	float playableHeight = l_windowH - l_topOffset - effTop - effBottom;

	if (playableWidth < 10.0f * l_blockSize || playableHeight < 10.0f * l_blockSize)
		return false;

	return true;
}

void Earthquake::GenerateCrackLines(const World& l_world, Window& l_window)
{
	m_cracks.clear();

	float winW = (float)l_window.GetWindowSize().x;
	float winH = (float)l_window.GetWindowSize().y;
	float topOffset = l_world.GetTopOffset();

	// Determine which borders are moving inward (positive delta = closing)
	bool closing[4] = { false, false, false, false };
	for (int i = 0; i < 4; i++)
	{
		if (m_pendingOffset[i] > l_world.GetBorderOffset(i))
			closing[i] = true;
	}

	int crackCount = RandomInt(6, 10);
	for (int c = 0; c < crackCount; c++)
	{
		// Pick a random closing border; if none closing, pick any
		int side;
		std::vector<int> closingSides;
		for (int i = 0; i < 4; i++)
			if (closing[i]) closingSides.push_back(i);

		if (!closingSides.empty())
			side = closingSides[RandomInt(0, (int)closingSides.size() - 1)];
		else
			side = RandomInt(0, 3);

		CrackLine crack;
		float length = RandomFloat(30.0f, 80.0f);
		crack.width = RandomFloat(2.0f, 4.0f);

		float effThick = l_world.GetEffectiveThickness(side);
		float effTop = l_world.GetEffectiveThickness(0);
		float effRight = l_world.GetEffectiveThickness(1);
		float effBottom = l_world.GetEffectiveThickness(2);
		float effLeft = l_world.GetEffectiveThickness(3);

		switch (side)
		{
			case 0: // Top border — cracks extend downward
			{
				float x = RandomFloat(effLeft, winW - effRight);
				crack.start = { x, topOffset + effThick };
				crack.end = { x + RandomFloat(-15.0f, 15.0f), topOffset + effThick + length };
				break;
			}
			case 1: // Right border — cracks extend leftward
			{
				float y = RandomFloat(topOffset + effTop, winH - effBottom);
				crack.start = { winW - effThick, y };
				crack.end = { winW - effThick - length, y + RandomFloat(-15.0f, 15.0f) };
				break;
			}
			case 2: // Bottom border — cracks extend upward
			{
				float x = RandomFloat(effLeft, winW - effRight);
				crack.start = { x, winH - effThick };
				crack.end = { x + RandomFloat(-15.0f, 15.0f), winH - effThick - length };
				break;
			}
			case 3: // Left border — cracks extend rightward
			{
				float y = RandomFloat(topOffset + effTop, winH - effBottom);
				crack.start = { effThick, y };
				crack.end = { effThick + length, y + RandomFloat(-15.0f, 15.0f) };
				break;
			}
			default:
				break;
		}

		m_cracks.push_back(crack);
	}
}

void Earthquake::Render(Window& l_window, const World& l_world)
{
	RenderTo(l_window.GetRenderWindow(), l_world);
}

void Earthquake::RenderTo(sf::RenderTarget& target, const World& /*l_world*/)
{
	if (m_cracks.empty() || m_crackAlpha <= 0.0f)
		return;
	sf::Uint8 alpha = (sf::Uint8)std::min(255.0f, std::max(0.0f, m_crackAlpha));

	// Ink-toned crack color (dark ink, not bright orange)
	sf::Color crackColor;
	if (m_state == QuakeState::Quaking && m_timer < 0.1f)
		crackColor = sf::Color(80, 30, 15, alpha); // Dark ember flash
	else
		crackColor = sf::Color(60, 20, 10, alpha); // Dark ink crack

	for (size_t c = 0; c < m_cracks.size(); c++)
	{
		const auto& crack = m_cracks[c];
		sf::Vector2f diff = crack.end - crack.start;
		float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
		if (len < 1.0f) continue;

		// Main crack as jagged polyline (5-7 points with perpendicular offsets)
		int segments = 5 + (int)(c % 3);
		sf::VertexArray jaggedLine(sf::LineStrip, (size_t)segments + 1);

		float perpX = -diff.y / len;
		float perpY = diff.x / len;

		for (int i = 0; i <= segments; i++)
		{
			float t = (float)i / (float)segments;
			float px = crack.start.x + diff.x * t;
			float py = crack.start.y + diff.y * t;

			// Perpendicular jagged offset (skip endpoints)
			if (i > 0 && i < segments)
			{
				float jag = InkRenderer::Hash((unsigned int)c, (unsigned int)i);
				jag = ((float)((int)jag % 2000) / 1000.0f - 1.0f) * 8.0f;
				px += perpX * jag;
				py += perpY * jag;
			}

			jaggedLine[(size_t)i].position = sf::Vector2f(px, py);
			jaggedLine[(size_t)i].color = crackColor;
		}
		target.draw(jaggedLine);

		// Branch crack (small side crack at ~40% along main)
		if (len > 30.0f)
		{
			float branchT = 0.3f + (float)(c % 4) * 0.1f;
			float bx = crack.start.x + diff.x * branchT;
			float by = crack.start.y + diff.y * branchT;
			float branchLen = len * 0.3f;

			float branchAngle = ((c % 2 == 0) ? 1.0f : -1.0f) * 0.7f;
			float bdx = (diff.x / len * std::cos(branchAngle) - diff.y / len * std::sin(branchAngle)) * branchLen;
			float bdy = (diff.x / len * std::sin(branchAngle) + diff.y / len * std::cos(branchAngle)) * branchLen;

			sf::Color branchColor(crackColor.r, crackColor.g, crackColor.b, (sf::Uint8)(alpha * 0.6f));
			InkRenderer::DrawWobblyLine(target, bx, by, bx + bdx, by + bdy,
										branchColor, 1.0f, 0.4f, (unsigned int)(c * 17), 4);
		}
	}
}

bool Earthquake::IsWarning() const
{
	return m_state == QuakeState::Warning;
}

bool Earthquake::JustQuaked()
{
	if (m_justQuaked)
	{
		m_justQuaked = false;
		return true;
	}
	return false;
}
