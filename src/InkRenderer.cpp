#include "InkRenderer.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

unsigned int InkRenderer::Hash(unsigned int l_a, unsigned int l_b)
{
	unsigned int h = l_a * 2654435761u ^ l_b * 2246822519u;
	h ^= h >> 13;
	h *= 0x5bd1e995u;
	h ^= h >> 15;
	return h;
}

float InkRenderer::Wobble(unsigned int l_seed, int l_index, float l_corruption)
{
	unsigned int h = Hash(l_seed, (unsigned int)l_index);
	// Map hash to [-1, 1] range
	float normalized = ((float)(h & 0xFFFF) / 32768.0f) - 1.0f;
	return normalized * l_corruption * 3.0f;
}

void InkRenderer::DrawWobblyRect(sf::RenderTarget& l_target,
								 float l_x, float l_y, float l_w, float l_h,
								 const sf::Color& l_fillColor,
								 const sf::Color& l_outlineColor,
								 float l_outlineThickness,
								 float l_corruption,
								 unsigned int l_seed)
{
	// Fill: a simple rectangle (hatching applied separately if needed)
	if (l_fillColor.a > 0)
	{
		sf::RectangleShape fill(sf::Vector2f(l_w, l_h));
		fill.setPosition(l_x, l_y);
		fill.setFillColor(l_fillColor);
		fill.setOutlineThickness(0);
		l_target.draw(fill);
	}

	// Outline: 4 wobbly lines forming the rectangle
	if (l_outlineColor.a > 0 && l_outlineThickness > 0)
	{
		// Top edge
		DrawWobblyLine(l_target, l_x, l_y, l_x + l_w, l_y,
					   l_outlineColor, l_outlineThickness, l_corruption, l_seed + 0, 6);
		// Right edge
		DrawWobblyLine(l_target, l_x + l_w, l_y, l_x + l_w, l_y + l_h,
					   l_outlineColor, l_outlineThickness, l_corruption, l_seed + 1, 6);
		// Bottom edge
		DrawWobblyLine(l_target, l_x + l_w, l_y + l_h, l_x, l_y + l_h,
					   l_outlineColor, l_outlineThickness, l_corruption, l_seed + 2, 6);
		// Left edge
		DrawWobblyLine(l_target, l_x, l_y + l_h, l_x, l_y,
					   l_outlineColor, l_outlineThickness, l_corruption, l_seed + 3, 6);
	}
}

void InkRenderer::DrawWobblyCircle(sf::RenderTarget& l_target,
								   float l_cx, float l_cy, float l_radius,
								   const sf::Color& l_fillColor,
								   const sf::Color& l_outlineColor,
								   float l_outlineThickness,
								   float l_corruption,
								   unsigned int l_seed,
								   int l_segments)
{
	if (l_segments < 6) l_segments = 6;

	// Build perturbed circle vertices
	std::vector<sf::Vector2f> points(l_segments);
	for (int i = 0; i < l_segments; i++)
	{
		float angle = (float)i / (float)l_segments * 2.0f * (float)M_PI;
		float r = l_radius + Wobble(l_seed, i, l_corruption);
		if (r < 1.0f) r = 1.0f;
		points[i] = sf::Vector2f(l_cx + std::cos(angle) * r,
								 l_cy + std::sin(angle) * r);
	}

	// Fill: TriangleFan
	if (l_fillColor.a > 0)
	{
		sf::VertexArray fan(sf::TriangleFan, l_segments + 2);
		fan[0].position = sf::Vector2f(l_cx, l_cy);
		fan[0].color = l_fillColor;
		for (int i = 0; i < l_segments; i++)
		{
			fan[i + 1].position = points[i];
			fan[i + 1].color = l_fillColor;
		}
		fan[l_segments + 1].position = points[0];
		fan[l_segments + 1].color = l_fillColor;
		l_target.draw(fan);
	}

	// Outline: LineStrip around perimeter
	if (l_outlineColor.a > 0 && l_outlineThickness > 0)
	{
		// For thicker outlines, draw multiple concentric rings
		int passes = std::max(1, (int)(l_outlineThickness));
		for (int p = 0; p < passes; p++)
		{
			float offsetR = (float)p * 0.5f;
			sf::VertexArray outline(sf::LineStrip, l_segments + 1);
			for (int i = 0; i < l_segments; i++)
			{
				float angle = (float)i / (float)l_segments * 2.0f * (float)M_PI;
				float dx = std::cos(angle) * offsetR;
				float dy = std::sin(angle) * offsetR;
				outline[i].position = sf::Vector2f(points[i].x + dx, points[i].y + dy);
				outline[i].color = l_outlineColor;
			}
			outline[l_segments].position = sf::Vector2f(
				points[0].x + std::cos(0.0f) * offsetR,
				points[0].y + std::sin(0.0f) * offsetR);
			outline[l_segments].color = l_outlineColor;
			l_target.draw(outline);
		}
	}
}

void InkRenderer::DrawWobblyLine(sf::RenderTarget& l_target,
								 float l_x1, float l_y1,
								 float l_x2, float l_y2,
								 const sf::Color& l_color,
								 float l_thickness,
								 float l_corruption,
								 unsigned int l_seed,
								 int l_segments)
{
	if (l_segments < 2) l_segments = 2;

	float dx = l_x2 - l_x1;
	float dy = l_y2 - l_y1;
	float len = std::sqrt(dx * dx + dy * dy);
	if (len < 0.001f) return;

	// Perpendicular direction for wobble
	float nx = -dy / len;
	float ny = dx / len;

	// Build the line as a quad strip for thickness
	sf::VertexArray quads(sf::TriangleStrip, (l_segments + 1) * 2);
	for (int i = 0; i <= l_segments; i++)
	{
		float t = (float)i / (float)l_segments;
		float px = l_x1 + dx * t;
		float py = l_y1 + dy * t;

		// Add perpendicular wobble (skip endpoints for cleaner joins)
		float wobbleOffset = 0.0f;
		if (i > 0 && i < l_segments)
			wobbleOffset = Wobble(l_seed, i, l_corruption);

		px += nx * wobbleOffset;
		py += ny * wobbleOffset;

		float halfThick = l_thickness * 0.5f;
		quads[i * 2].position = sf::Vector2f(px + nx * halfThick, py + ny * halfThick);
		quads[i * 2].color = l_color;
		quads[i * 2 + 1].position = sf::Vector2f(px - nx * halfThick, py - ny * halfThick);
		quads[i * 2 + 1].color = l_color;
	}
	l_target.draw(quads);
}

void InkRenderer::DrawCrossHatch(sf::RenderTarget& l_target,
								 float l_x, float l_y, float l_w, float l_h,
								 const sf::Color& l_color,
								 float l_spacing,
								 float l_angleDeg,
								 float l_corruption,
								 unsigned int l_seed)
{
	if (l_spacing < 1.0f) l_spacing = 1.0f;

	float angleRad = l_angleDeg * (float)M_PI / 180.0f;
	float cosA = std::cos(angleRad);
	float sinA = std::sin(angleRad);

	// Diagonal extent for line coverage
	float diag = std::sqrt(l_w * l_w + l_h * l_h);
	int numLines = (int)(diag / l_spacing) + 2;

	for (int i = -numLines / 2; i <= numLines / 2; i++)
	{
		// Line center offset perpendicular to angle direction
		float offset = (float)i * l_spacing;
		float cx = l_x + l_w * 0.5f;
		float cy = l_y + l_h * 0.5f;

		// Perpendicular offset
		float px = cx + (-sinA) * offset;
		float py = cy + cosA * offset;

		// Line endpoints extending beyond the rect
		float halfDiag = diag * 0.6f;
		float x1 = px - cosA * halfDiag;
		float y1 = py - sinA * halfDiag;
		float x2 = px + cosA * halfDiag;
		float y2 = py + sinA * halfDiag;

		float wobble1 = Wobble(l_seed, i * 2, l_corruption * 0.3f);
		float wobble2 = Wobble(l_seed, i * 2 + 1, l_corruption * 0.3f);

		float fx1 = x1 + (-sinA) * wobble1;
		float fy1 = y1 + cosA * wobble1;
		float fx2 = x2 + (-sinA) * wobble2;
		float fy2 = y2 + cosA * wobble2;

		// Clamp endpoints to rect bounds to prevent bleeding into adjacent areas
		fx1 = std::max(l_x, std::min(l_x + l_w, fx1));
		fy1 = std::max(l_y, std::min(l_y + l_h, fy1));
		fx2 = std::max(l_x, std::min(l_x + l_w, fx2));
		fy2 = std::max(l_y, std::min(l_y + l_h, fy2));

		sf::Vertex line[] = {
			sf::Vertex(sf::Vector2f(fx1, fy1), l_color),
			sf::Vertex(sf::Vector2f(fx2, fy2), l_color)
		};
		l_target.draw(line, 2, sf::Lines);
	}
}

void InkRenderer::DrawDashedArc(sf::RenderTarget& l_target,
								float l_cx, float l_cy, float l_radius,
								float l_fraction,
								const sf::Color& l_color,
								float l_corruption,
								unsigned int l_seed)
{
	if (l_fraction <= 0.0f) return;
	if (l_fraction > 1.0f) l_fraction = 1.0f;

	const float dashAngleDeg = 8.0f;
	const float gapAngleDeg = 4.0f;
	const float totalAngle = 360.0f * l_fraction;
	float currentAngle = -90.0f; // Start from top

	int dashIndex = 0;
	while (currentAngle < -90.0f + totalAngle)
	{
		float dashEnd = currentAngle + dashAngleDeg;
		if (dashEnd > -90.0f + totalAngle) dashEnd = -90.0f + totalAngle;

		// Draw this dash as a series of line segments
		int segs = 4;
		sf::VertexArray dash(sf::LineStrip, segs + 1);
		for (int i = 0; i <= segs; i++)
		{
			float t = (float)i / (float)segs;
			float a = (currentAngle + t * (dashEnd - currentAngle)) * (float)M_PI / 180.0f;
			float r = l_radius + Wobble(l_seed, dashIndex * 10 + i, l_corruption * 0.5f);
			dash[i].position = sf::Vector2f(l_cx + std::cos(a) * r, l_cy + std::sin(a) * r);
			dash[i].color = l_color;
		}
		l_target.draw(dash);

		currentAngle = dashEnd + gapAngleDeg;
		dashIndex++;
	}
}

void InkRenderer::DrawTornEdge(sf::RenderTarget& l_target,
							   float l_x, float l_y, float l_length,
							   bool l_horizontal, bool l_flipDirection,
							   const sf::Color& l_color,
							   float l_corruption,
							   unsigned int l_seed)
{
	float spacing = 6.0f;
	int count = (int)(l_length / spacing);
	if (count < 1) return;

	for (int i = 0; i < count; i++)
	{
		float tearHeight = 2.0f + std::abs(Wobble(l_seed, i, l_corruption + 0.3f));
		float dir = l_flipDirection ? -1.0f : 1.0f;

		sf::ConvexShape tri;
		tri.setPointCount(3);
		tri.setFillColor(l_color);

		if (l_horizontal)
		{
			float bx = l_x + i * spacing;
			tri.setPoint(0, sf::Vector2f(bx, l_y));
			tri.setPoint(1, sf::Vector2f(bx + spacing, l_y));
			tri.setPoint(2, sf::Vector2f(bx + spacing * 0.5f, l_y + tearHeight * dir));
		}
		else
		{
			float by = l_y + i * spacing;
			tri.setPoint(0, sf::Vector2f(l_x, by));
			tri.setPoint(1, sf::Vector2f(l_x, by + spacing));
			tri.setPoint(2, sf::Vector2f(l_x + tearHeight * dir, by + spacing * 0.5f));
		}
		l_target.draw(tri);
	}
}

void InkRenderer::DrawStar(sf::RenderTarget& l_target,
							float l_cx, float l_cy, float l_outerRadius,
							const sf::Color& l_fillColor,
							const sf::Color& l_outlineColor,
							float l_corruption,
							bool l_filled,
							unsigned int l_seed)
{
	const int points = 5;
	const int totalVerts = points * 2;
	float innerRadius = l_outerRadius * 0.4f;

	sf::ConvexShape star;
	star.setPointCount(totalVerts);

	for (int i = 0; i < totalVerts; i++)
	{
		float angle = ((float)i / (float)totalVerts) * 2.0f * (float)M_PI - (float)M_PI / 2.0f;
		float r = (i % 2 == 0) ? l_outerRadius : innerRadius;
		r += Wobble(l_seed, i, l_corruption);
		if (r < 1.0f) r = 1.0f;
		star.setPoint(i, sf::Vector2f(l_cx + std::cos(angle) * r,
									  l_cy + std::sin(angle) * r));
	}

	if (l_filled)
	{
		star.setFillColor(l_fillColor);
		star.setOutlineColor(l_outlineColor);
		star.setOutlineThickness(1.0f);
	}
	else
	{
		star.setFillColor(sf::Color::Transparent);
		star.setOutlineColor(l_outlineColor);
		star.setOutlineThickness(1.5f);
	}

	l_target.draw(star);
}
