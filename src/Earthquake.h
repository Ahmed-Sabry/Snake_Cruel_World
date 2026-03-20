#pragma once

#include "Window.h"
#include "Snake.h"
#include <vector>

class World;

enum class QuakeState
{
	Idle,
	Warning,
	Quaking
};

struct CrackLine
{
	sf::Vector2f start;
	sf::Vector2f end;
	float width;
};

class Earthquake
{
public:
	Earthquake();

	void Reset(float l_blockSize);
	void Update(float l_dt, World& l_world, Window& l_window);
	void Render(Window& l_window, const World& l_world);
	void RenderTo(sf::RenderTarget& l_target, const World& l_world);

	bool IsWarning() const;
	bool JustQuaked();

private:
	void GenerateQuake(const World& l_world, float l_windowWidth, float l_windowHeight);
	void ApplyQuake(World& l_world, Window& l_window);
	void GenerateCrackLines(const World& l_world, Window& l_window);
	bool ValidateOffsets(const float l_offsets[4], float l_baseThickness,
						 float l_windowW, float l_windowH, float l_topOffset, float l_blockSize) const;

	QuakeState m_state;
	float m_timer;
	float m_quakeInterval;
	float m_warningDuration;
	float m_blockSize;

	float m_pendingOffset[4]; // offsets to apply: 0=top, 1=right, 2=bottom, 3=left

	bool m_justQuaked;

	std::vector<CrackLine> m_cracks;
	float m_crackAlpha;
	sf::RectangleShape m_crackRect;
};
