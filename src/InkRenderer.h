#pragma once

#include "Platform/Platform.hpp"
#include "Window.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Utility class for drawing hand-drawn "ink" style shapes.
// All shapes use a corruption factor (0.0 - 1.0) to control wobble amplitude,
// creating progressively more unhinged-looking visuals.
class InkRenderer
{
public:
	// Draw a wobbly-outlined rectangle (hand-drawn feel)
	// Fills with color, outlines with outlineColor at given thickness
	static void DrawWobblyRect(sf::RenderTarget& l_target,
							   float l_x, float l_y, float l_w, float l_h,
							   const sf::Color& l_fillColor,
							   const sf::Color& l_outlineColor,
							   float l_outlineThickness,
							   float l_corruption,
							   unsigned int l_seed = 0);

	// Draw a wobbly-outlined circle (hand-drawn feel)
	// Uses a TriangleFan vertex array with perturbed radii
	static void DrawWobblyCircle(sf::RenderTarget& l_target,
								 float l_cx, float l_cy, float l_radius,
								 const sf::Color& l_fillColor,
								 const sf::Color& l_outlineColor,
								 float l_outlineThickness,
								 float l_corruption,
								 unsigned int l_seed = 0,
								 int l_segments = 20);

	// Draw a wobbly line between two points
	static void DrawWobblyLine(sf::RenderTarget& l_target,
							   float l_x1, float l_y1,
							   float l_x2, float l_y2,
							   const sf::Color& l_color,
							   float l_thickness,
							   float l_corruption,
							   unsigned int l_seed = 0,
							   int l_segments = 8);

	// Draw cross-hatching pattern inside a rectangular area
	// angle: 0 = horizontal, 45 = diagonal, etc.
	static void DrawCrossHatch(sf::RenderTarget& l_target,
							   float l_x, float l_y, float l_w, float l_h,
							   const sf::Color& l_color,
							   float l_spacing,
							   float l_angleDeg,
							   float l_corruption,
							   unsigned int l_seed = 0);

	// Draw a dashed arc (used for timed apple ring)
	static void DrawDashedArc(sf::RenderTarget& l_target,
							  float l_cx, float l_cy, float l_radius,
							  float l_fraction, // 0.0 to 1.0
							  const sf::Color& l_color,
							  float l_thickness,
							  float l_corruption,
							  unsigned int l_seed = 0);

	// Draw small "torn paper" triangles along an edge
	static void DrawTornEdge(sf::RenderTarget& l_target,
							 float l_x, float l_y, float l_length,
							 bool l_horizontal, bool l_flipDirection,
							 const sf::Color& l_color,
							 float l_corruption,
							 unsigned int l_seed = 0);

	// Draw a small hand-drawn star shape
	static void DrawStar(sf::RenderTarget& l_target,
						 float l_cx, float l_cy, float l_outerRadius,
						 const sf::Color& l_fillColor,
						 const sf::Color& l_outlineColor,
						 float l_corruption,
						 bool l_filled,
						 unsigned int l_seed = 0);

	// Deterministic wobble offset based on seed and index
	static float Wobble(unsigned int l_seed, int l_index, float l_corruption);
	// Simple hash for seeding (public for use by other ink-style renderers)
	static unsigned int Hash(unsigned int l_a, unsigned int l_b);
};
