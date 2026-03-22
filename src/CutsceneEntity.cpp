#include "CutsceneEntity.h"
#include "InkRenderer.h"
#include <algorithm>
#include <cmath>

// File-scope statics for rotated-entity render texture (only grows; released via ReleaseStaticResources)
static sf::RenderTexture s_rt;
static unsigned int s_rtW = 0, s_rtH = 0;
static bool s_rtValid = false;

void CutsceneEntity::Render(sf::RenderTarget& l_target, const sf::Font& l_font) const
{
	if (!visible)
		return;

	// Non-Text shapes with non-zero rotation need to be drawn via a temp texture
	// since InkRenderer draws with absolute coordinates and doesn't support transforms
	if (rotation != 0.f && shape != EntityShape::Text && shape != EntityShape::None)
	{
		RenderRotated(l_target, l_font);
		return;
	}

	RenderDirect(l_target, l_font);
}

void CutsceneEntity::RenderDirect(sf::RenderTarget& l_target, const sf::Font& l_font) const
{
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
		sf::Color fillCol = filled ? drawColor : sf::Color::Transparent;
		sf::Color outCol = filled
			? sf::Color((sf::Uint8)(drawColor.r * 0.6f), (sf::Uint8)(drawColor.g * 0.6f),
						(sf::Uint8)(drawColor.b * 0.6f), drawColor.a)
			: drawColor;
		InkRenderer::DrawWobblyRect(l_target,
									drawX, drawY, w, h,
									fillCol, outCol,
									filled ? 1.f : 2.f, corruption, seed);
		if (hasEyes && filled)
		{
			float cx = drawX + w * 0.5f;
			float cy = drawY + h * 0.5f;
			float eyeR = std::max(2.5f, w * 0.12f);
			float pupilR = std::max(1.0f, eyeR * 0.4f);
			float fwd = w * 0.2f;
			float side = h * 0.2f;
			// Eyes face right
			float ex1 = cx + fwd, ey1 = cy - side;
			float ex2 = cx + fwd, ey2 = cy + side;

			sf::CircleShape eye(eyeR);
			eye.setOrigin(eyeR, eyeR);
			eye.setFillColor(outCol);
			eye.setPosition(ex1, ey1);
			l_target.draw(eye);
			eye.setPosition(ex2, ey2);
			l_target.draw(eye);

			sf::CircleShape pupil(pupilR);
			pupil.setOrigin(pupilR, pupilR);
			pupil.setFillColor(sf::Color(255, 255, 255, drawColor.a));
			pupil.setPosition(ex1, ey1);
			l_target.draw(pupil);
			pupil.setPosition(ex2, ey2);
			l_target.draw(pupil);
		}
		break;
	}
	case EntityShape::Circle:
	{
		float r = radius * scale.x;

		if (isApple)
		{
			// Breathing animation (period ≈ 2s)
			float t = spawnClock.getElapsedTime().asSeconds();
			r *= 1.0f + std::sin(t * static_cast<float>(M_PI)) * 0.05f;

			// Ink-tint outline (matches gameplay)
			sf::Color inkOutline(45, 40, 55, (sf::Uint8)(drawColor.a * 0.78f));
			InkRenderer::DrawWobblyCircle(l_target,
										  position.x, position.y, r,
										  drawColor, inkOutline,
										  1.5f, corruption, seed, 16);

			// Leaf (V shape at top) — scaled to radius
			float leafLen = r * 0.5f;
			float leafSpread = r * 0.3f;
			sf::Color leafCol(45, 40, 55, (sf::Uint8)(drawColor.a * 0.7f));
			InkRenderer::DrawWobblyLine(l_target,
										position.x, position.y - r,
										position.x + leafSpread, position.y - r - leafLen,
										leafCol, 1.f, corruption * 0.5f, seed + 1);
			InkRenderer::DrawWobblyLine(l_target,
										position.x, position.y - r,
										position.x - leafSpread * 0.7f, position.y - r - leafLen * 0.8f,
										leafCol, 1.f, corruption * 0.5f, seed + 2);

			// Highlight dot — scaled to radius
			float hlR = std::max(1.f, r * 0.15f);
			sf::CircleShape hl(hlR);
			hl.setOrigin(hlR, hlR);
			hl.setPosition(position.x - r * 0.3f, position.y - r * 0.3f);
			hl.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(drawColor.a * 0.31f)));
			l_target.draw(hl);
		}
		else
		{
			sf::Color fillCol = filled ? drawColor : sf::Color::Transparent;
			sf::Color outCol = filled
				? sf::Color((sf::Uint8)(drawColor.r * 0.6f), (sf::Uint8)(drawColor.g * 0.6f),
							(sf::Uint8)(drawColor.b * 0.6f), drawColor.a)
				: drawColor;
			InkRenderer::DrawWobblyCircle(l_target,
										  position.x, position.y, r,
										  fillCol, outCol,
										  filled ? 1.5f : 2.f, corruption, seed);
		}
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

void CutsceneEntity::RenderRotated(sf::RenderTarget& l_target, const sf::Font& l_font) const
{
	// Compute the actual rendered size and world center for this shape
	float padding = 10.f + corruption * 20.f;
	float localW, localH, worldCX, worldCY;

	switch (shape)
	{
	case EntityShape::Rect:
	{
		float sw = width * scale.x;
		float sh = height * scale.y;
		localW = std::abs(sw) + padding * 2.f;
		localH = std::abs(sh) + padding * 2.f;
		worldCX = position.x + width * 0.5f;
		worldCY = position.y + height * 0.5f;
		break;
	}
	case EntityShape::Circle:
	case EntityShape::Star:
	{
		float r = radius * std::abs(scale.x);
		localW = localH = r * 2.f + padding * 2.f;
		worldCX = position.x;
		worldCY = position.y;
		break;
	}
	case EntityShape::Line:
	{
		float ex = width * scale.x;
		float ey = height * scale.y;
		localW = std::abs(ex) + padding * 2.f;
		localH = std::abs(ey) + padding * 2.f;
		worldCX = position.x + ex * 0.5f;
		worldCY = position.y + ey * 0.5f;
		break;
	}
	default:
		return;
	}

	unsigned int tw = (unsigned int)std::ceil(localW);
	unsigned int th = (unsigned int)std::ceil(localH);
	if (tw == 0 || th == 0) return;

	// Reuse file-scope render texture (resized only when needed)
	if (tw > s_rtW || th > s_rtH)
	{
		unsigned int prevW = s_rtW, prevH = s_rtH;
		s_rtW = std::max(tw, s_rtW);
		s_rtH = std::max(th, s_rtH);
		if (!s_rt.create(s_rtW, s_rtH))
		{
			s_rtW = prevW;
			s_rtH = prevH;
			s_rtValid = false;
			return;
		}
		s_rtValid = true;
	}
	if (!s_rtValid) return;
	s_rt.clear(sf::Color::Transparent);

	// Draw a non-rotated copy into the temp texture at local coordinates
	CutsceneEntity temp = *this;
	temp.rotation = 0.f;

	switch (shape)
	{
	case EntityShape::Rect:
	{
		// Set temp to draw at (padding, padding) with pre-scaled dimensions
		float sw = width * scale.x;
		float sh = height * scale.y;
		temp.width = std::abs(sw);
		temp.height = std::abs(sh);
		temp.scale = {1.f, 1.f};
		temp.position = {padding, padding};
		break;
	}
	case EntityShape::Circle:
	case EntityShape::Star:
	{
		temp.radius = radius * std::abs(scale.x);
		temp.scale = {1.f, 1.f};
		temp.position = {localW * 0.5f, localH * 0.5f};
		break;
	}
	case EntityShape::Line:
	{
		float ex = width * scale.x;
		float ey = height * scale.y;
		temp.width = ex;
		temp.height = ey;
		temp.scale = {1.f, 1.f};
		temp.position = {padding + std::max(0.f, -ex), padding + std::max(0.f, -ey)};
		break;
	}
	default:
		break;
	}

	temp.RenderDirect(s_rt, l_font);
	s_rt.display();

	// Draw the texture rotated around its center
	sf::Sprite spr(s_rt.getTexture());
	spr.setTextureRect(sf::IntRect(0, 0, tw, th));
	spr.setOrigin(localW * 0.5f, localH * 0.5f);
	spr.setPosition(worldCX, worldCY);
	spr.setRotation(rotation);
	l_target.draw(spr);
}

void CutsceneEntity::ReleaseStaticResources()
{
	if (s_rtW > 0 || s_rtH > 0)
	{
		s_rt.create(1, 1);
		s_rtW = 0;
		s_rtH = 0;
	}
	s_rtValid = false;
}
