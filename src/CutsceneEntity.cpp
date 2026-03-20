#include "CutsceneEntity.h"
#include "InkRenderer.h"

void CutsceneEntity::Render(sf::RenderTarget& l_target, const sf::Font& l_font) const
{
	if (!visible)
		return;

	sf::Color drawColor = color;
	drawColor.a = (sf::Uint8)std::max(0.f, std::min(255.f, alpha));

	switch (shape)
	{
	case EntityShape::Rect:
	{
		float w = width * scale.x;
		float h = height * scale.y;
		// Scale from center: offset position so center stays fixed
		float drawX = position.x + (width - w) * 0.5f;
		float drawY = position.y + (height - h) * 0.5f;
		InkRenderer::DrawWobblyRect(l_target,
									drawX, drawY, w, h,
									sf::Color::Transparent, drawColor,
									2.f, corruption, seed);
		break;
	}
	case EntityShape::Circle:
	{
		// Circle center is already at position — scale only affects radius
		float r = radius * scale.x;
		InkRenderer::DrawWobblyCircle(l_target,
									  position.x, position.y, r,
									  sf::Color::Transparent, drawColor,
									  2.f, corruption, seed);
		break;
	}
	case EntityShape::Star:
	{
		float r = radius * scale.x;
		InkRenderer::DrawStar(l_target,
							  position.x, position.y, r,
							  drawColor, drawColor,
							  corruption, true, seed);
		break;
	}
	case EntityShape::Line:
	{
		float x2 = position.x + width * scale.x;
		float y2 = position.y + height * scale.y;
		InkRenderer::DrawWobblyLine(l_target,
									position.x, position.y, x2, y2,
									drawColor, 2.f, corruption, seed);
		break;
	}
	case EntityShape::Text:
	{
		sf::Text textObj;
		textObj.setFont(l_font);
		textObj.setString(text);
		textObj.setCharacterSize(charSize);
		textObj.setFillColor(drawColor);
		textObj.setPosition(position);
		textObj.setRotation(rotation);
		textObj.setScale(scale);
		l_target.draw(textObj);
		break;
	}
	case EntityShape::None:
	default:
		break;
	}
}
