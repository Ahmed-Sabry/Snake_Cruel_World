#pragma once

#include "Platform/Platform.hpp"
#include <string>
#include <vector>
#include <functional>

enum class EntityShape { None, Rect, Circle, Line, Star, Text, Sprite };

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

	// Sprite support
	std::string texturePath;
	sf::Texture texture;
	bool textureLoaded = false;
	bool flipX = false;
	bool flipY = false;

	bool LoadTexture();

	// Parent-child grouping (empty = root entity)
	std::string parent;
	mutable const CutsceneEntity* parentPtr = nullptr; // cached; resolved by CutsceneScene

	// Per-entity update callbacks (used by ExpressionAction)
	// Signature: void(float dt, float totalTime)
	std::vector<std::function<void(float, float)>> updateCallbacks;
	float totalTime = 0.f;
	void ClearUpdateCallbacks() { updateCallbacks.clear(); }

	// State
	bool visible = true;
	bool filled = false;
	bool hasEyes = false;
	bool isApple = false;
	int zOrder = 0;
	mutable sf::Clock spawnClock;

	void Render(sf::RenderTarget& l_target, const sf::Font& l_font,
				const sf::Transform& l_parentTransform = sf::Transform::Identity) const;

	static void ReleaseStaticResources();

private:
	void RenderDirect(sf::RenderTarget& l_target, const sf::Font& l_font,
					  const sf::Transform& l_parentTransform) const;
	void RenderRotated(sf::RenderTarget& l_target, const sf::Font& l_font,
					   const sf::Transform& l_parentTransform) const;
	void RenderSprite(sf::RenderTarget& l_target,
					  const sf::Transform& l_parentTransform) const;
};
