#include "CutsceneScene.h"
#include <algorithm>

CutsceneEntity& CutsceneScene::Spawn(const std::string& l_name, EntityShape l_shape)
{
	// Remove any existing entity with this name to prevent duplicates
	Destroy(l_name);

	CutsceneEntity entity;
	entity.name = l_name;
	entity.shape = l_shape;
	m_entities.push_back(std::move(entity));
	m_sortDirty = true;
	return m_entities.back();
}

CutsceneEntity* CutsceneScene::Get(const std::string& l_name)
{
	for (auto& e : m_entities)
	{
		if (e.name == l_name)
			return &e;
	}
	return nullptr;
}

void CutsceneScene::Destroy(const std::string& l_name)
{
	auto it = std::remove_if(m_entities.begin(), m_entities.end(),
				   [&](const CutsceneEntity& e) { return e.name == l_name; });
	if (it != m_entities.end())
	{
		m_entities.erase(it, m_entities.end());
		m_sortDirty = true;
	}
}

void CutsceneScene::Clear()
{
	m_entities.clear();
	m_sortDirty = false;
}

void CutsceneScene::Render(sf::RenderTarget& l_target, const sf::Font& l_font)
{
	if (m_sortDirty)
	{
		std::stable_sort(m_entities.begin(), m_entities.end(),
						 [](const CutsceneEntity& a, const CutsceneEntity& b)
						 { return a.zOrder < b.zOrder; });
		m_sortDirty = false;
	}

	for (const auto& entity : m_entities)
		entity.Render(l_target, l_font);
}
