#include "bosses/PlaceholderBoss.h"

#include "World.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cmath>

PlaceholderBoss::PlaceholderBoss(const BossConfig& l_config)
	: Boss(l_config)
{
}

void PlaceholderBoss::RenderTo(sf::RenderTarget& l_target, float l_gameTime, const BossContext& l_ctx) const
{
	if (GetLifecycleState() == BossLifecycleState::Dormant || l_ctx.world == nullptr)
		return;

	const float blockSize = (l_ctx.blockSize > 0.0f) ? l_ctx.blockSize : 16.0f;
	const float left = l_ctx.world->GetEffectiveThickness(3) + blockSize * 0.5f;
	const float right = l_ctx.world->GetMaxX() * blockSize -
		l_ctx.world->GetEffectiveThickness(1) - blockSize * 0.5f;
	const float top = l_ctx.world->GetTopOffset() + l_ctx.world->GetEffectiveThickness(0) +
		blockSize * 1.5f;
	const float centerX = (left + right) * 0.5f;
	const float pulse = 1.0f + std::sin(l_gameTime * 3.0f) * 0.08f;
	const float radius = blockSize * 1.35f * pulse;

	sf::CircleShape shell(radius);
	shell.setOrigin(radius, radius);
	shell.setPosition(centerX, top);
	shell.setFillColor(sf::Color(34, 26, 42, 70));
	shell.setOutlineThickness(2.0f);
	shell.setOutlineColor(IsDefeated()
		? sf::Color(70, 150, 90, 220)
		: sf::Color(165, 72, 58, 220));
	l_target.draw(shell);

	const int progressMax = std::max(1, GetProgressMax());
	const float pipSpacing = blockSize * 0.8f;
	const float pipStartX = centerX - ((progressMax - 1) * pipSpacing * 0.5f);
	for (int i = 0; i < progressMax; ++i)
	{
		sf::RectangleShape pip(sf::Vector2f(blockSize * 0.45f, blockSize * 0.45f));
		pip.setOrigin(pip.getSize().x * 0.5f, pip.getSize().y * 0.5f);
		pip.setPosition(pipStartX + i * pipSpacing, top + radius + blockSize * 0.9f);
		pip.setFillColor(i < GetProgressCurrent()
			? sf::Color(215, 110, 80, 230)
			: sf::Color(72, 58, 66, 150));
		l_target.draw(pip);
	}
}
