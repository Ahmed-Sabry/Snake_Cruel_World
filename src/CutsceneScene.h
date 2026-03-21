#pragma once

#include "CutsceneEntity.h"
#include <vector>
#include <string>

class CutsceneScene
{
public:
	CutsceneEntity& Spawn(const std::string& l_name, EntityShape l_shape);
	CutsceneEntity* Get(const std::string& l_name);
	void Destroy(const std::string& l_name);
	void Clear();

	void Render(sf::RenderTarget& l_target, const sf::Font& l_font);

private:
	std::vector<CutsceneEntity> m_entities;
	bool m_sortDirty = false;
};
