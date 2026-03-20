#pragma once

#include "Window.h"
#include "Snake.h"
#include <vector>

struct QuicksandPatch
{
	int x, y; // top-left corner (grid coords) of 3x3 patch
};

class Quicksand
{
public:
	Quicksand();

	void Reset(float l_maxX, float l_maxY, float l_borderThickness, float l_blockSize, float l_topOffset);
	void Update(float l_dt, float l_maxX, float l_maxY, float l_borderThickness, float l_blockSize, float l_topOffset);
	void Render(Window& l_window, float l_blockSize);
	void RenderTo(sf::RenderTarget& l_target, float l_blockSize);
	bool IsOnQuicksand(const Position& l_pos) const;

private:
	void GeneratePatches(float l_maxX, float l_maxY, float l_borderThickness, float l_blockSize, float l_topOffset);

	std::vector<QuicksandPatch> m_patches;
	float m_relocateTimer;
	float m_relocateInterval;
	int m_patchCount;

	sf::RectangleShape m_patchRect;
};
