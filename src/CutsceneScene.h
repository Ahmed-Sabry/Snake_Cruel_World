#pragma once

#include "CutsceneEntity.h"
#include <vector>
#include <string>
#include <unordered_set>

class CutsceneScene
{
public:
	CutsceneEntity& Spawn(const std::string& l_name, EntityShape l_shape);
	CutsceneEntity* Get(const std::string& l_name);
	const CutsceneEntity* Get(const std::string& l_name) const;
	void Destroy(const std::string& l_name);
	void Clear();

	void Update(float l_dt);
	void Render(sf::RenderTarget& l_target, const sf::Font& l_font);
	void ResolveParentPointers();

	sf::Transform GetWorldTransform(const std::string& l_name) const;

private:
	sf::Transform BuildWorldTransform(const CutsceneEntity& l_entity,
									  std::unordered_set<const CutsceneEntity*>& l_visited) const;
	const CutsceneEntity* GetConst(const std::string& l_name) const;

	std::vector<CutsceneEntity> m_entities;
	bool m_sortDirty = false;
};
