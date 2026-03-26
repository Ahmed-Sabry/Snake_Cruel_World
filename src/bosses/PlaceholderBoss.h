#pragma once

#include "Boss.h"

class PlaceholderBoss : public Boss
{
public:
	explicit PlaceholderBoss(const BossConfig& l_config);

	void RenderTo(sf::RenderTarget& l_target, float l_gameTime, const BossContext& l_ctx) const override;
};
