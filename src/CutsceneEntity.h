#pragma once

#include "Platform/Platform.hpp"
#include <string>

enum class EntityShape { None, Rect, Circle, Line, Star, Text };

struct CutsceneEntity
{
	std::string name;
	EntityShape shape = EntityShape::None;

	// Animatable properties
	sf::Vector2f position = {0.f, 0.f};
	sf::Vector2f scale = {1.f, 1.f};
	float rotation = 0.f;
	float alpha = 255.f;
	sf::Color color = sf::Color(60, 50, 45);

	// Shape-specific params
	float width = 0.f, height = 0.f;
	float radius = 0.f;
	float corruption = 0.1f;
	unsigned int seed = 0;
	std::string text;
	unsigned int charSize = 28;

	// Line-specific (x2, y2 stored as width/height offset from position)
	// Line goes from position to (position.x + width, position.y + height)

	// State
	bool visible = true;
	int zOrder = 0;

	void Render(sf::RenderTarget& l_target, const sf::Font& l_font) const;

	static void ReleaseStaticResources();

private:
	void RenderDirect(sf::RenderTarget& l_target, const sf::Font& l_font) const;
	void RenderRotated(sf::RenderTarget& l_target, const sf::Font& l_font) const;
};
